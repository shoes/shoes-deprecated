# encoding: UTF-8

# 
# Credits, inspiration goes to : 
# https://github.com/emilsoman/diy_prof/tree/dot-reporter
# Also, thanks to @passenger94 at github for showing what could be done
# and how to do it.

# This uses the url/visit method of structuring a Shoes app and those
# Windows/Scripts need a common data source. I don't trust @@ vars so
# I'm going to keep common things in a global $shoe_profiler class.
# For better or for worse.


module TimeHelpers
    # These methods make use of `clock_gettime` method introduced in Ruby 2.1
    # to measure CPU time and Wall clock time. (microsecond is second / 1 000 000)
    def cpu_time
        Process.clock_gettime(Process::CLOCK_PROCESS_CPUTIME_ID, :microsecond)
    end

    def wall_time
        Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
    end
end

class Tracer
    include TimeHelpers
    EVENTS = [:call, :return, :c_call, :c_return]
    
    def initialize(reporter, c_calls)
        @reporter = reporter
        events = c_calls ? EVENTS : EVENTS - [:c_call, :c_return]
        
        @tracepoints = events.map do |event|
            TracePoint.new(event) do |trace|
                reporter.record(event, trace.method_id, cpu_time)
            end
        end
        
    end
    
    def enable; @tracepoints.each(&:enable) end
    def disable; @tracepoints.each(&:disable) end
    def result; @reporter.result end
end


CallInfo = Struct.new(:name, :time)
MethodInfo = Struct.new(:count, :total_time, :self_time)

class Reporter
    def initialize
        # A stack for pushing/popping methods when methods get called/returned
        @call_stack = []
        # Nodes for all methods
        @methods = {}
        # Connections between the nodes
        @calls = {}
    end

    def record(event, method_name, time)
        case event
        when :call, :c_call
            @call_stack << CallInfo.new(method_name, time)
        when :return, :c_return
            # Return cannot be the first event in the call stack
            return if @call_stack.empty?

            method = @call_stack.pop
            # Set execution time of method in call info
            method.time = time - method.time
            
            add_method_to_call_tree(method)
        end
    end
    
    def result
        [@methods, @calls]
    end

    private
    
    def add_method_to_call_tree(method)
        # Add method as a node to the call graph
        @methods[method.name] ||= MethodInfo.new(0, 0, 0)
        # Update total time(spent inside the method and methods called inside this method)
        @methods[method.name].total_time += method.time
        # Update self time(spent inside the method and not methods called inside this method)
        # This will be subtracted when children are added to the graph
        @methods[method.name].self_time += method.time
        # Update total no of times the method was called
        @methods[method.name].count += 1

        # If the method has a parent in the call stack
        # Add a connection from the parent node to this method
        if parent = @call_stack.last
            @calls[parent.name] ||= {}
            @calls[parent.name][method.name] ||= 0
            @calls[parent.name][method.name] += 1

            # Take away self time of parent
            @methods[parent.name] ||= MethodInfo.new(0, 0, 0)
            @methods[parent.name].self_time -= method.time
        end
    end

end


class RadioLabel < Shoes::Widget
    
    def initialize(options={})
        label = options[:text] || ""
        active = options[:active] || false
        inner_margins = options[:inner_margins] || [0,0,0,0]
        r_margins = inner_margins.dup; r_margins[2] = 5 
        p_margins = [0,3] + inner_margins[2..3]
        
        @r = radio checked: active, margin: r_margins
        @p = para label, margin: p_margins
    end
    
    def checked?; @r.checked? end
    def checked=(bool); @r.checked = bool end
end

class NodeWidget < Shoes::Widget
    def initialize(options={})
        name, method_info = options[:node]
        size = options[:size]*8
        ca = options[:color_alpha]
        infos = options[:info]
        self.width = size+4; self.height = size/2+4
        
        stack width: size+4, height: size/2+4 do
            oval 2,2, size, size/2, fill: ca == 1.0 ? red : red(ca)
            @n = inscription "#{name}\n#{infos}", font: "mono", align: "center", margin: 0, displace_top: (size/4)-8
        end
    end
end

$shoes_profiler = nil;
class ProfilerDB 
  attr_accessor :nodes, :links, :add_c_calls, :file
  nodes = {}
  links = {}
  file = ''
  def start
    @tracer = nil if @tracer
    @tracer = Tracer.new(Reporter.new, add_c_calls)
        
    @tracer.enable
    yield
  end
  def stop
    @tracer.disable
    @tracer.result
  end
end



class DiyProf < Shoes
  # TODO find how to construct a graph with connected nodes (like Graphviz) ...
  $shoes_profiler = ProfilerDB.new()
  $shoes_profiler.file = $shoes_examine_file # passed on the commandline with '-e file' or nil
  #$stderr.puts "got it: #{$shoes_profiler.file}"
  url "/", :index
  url  "/graphical", :graphscreen
  url "/terminal", :textscreen
  def index
    @file = $shoes_profiler.file
    stack do
      para "Select the starting script for your app. If you choose the GUI ",
        "display you may want to expand this Window first. ",
        "You MUST end profiling manually by clicking the 'End Profiling' button" 
      para "Once profiling is stopped, you can examine the results"
      flow do
        flow {@gui_display = check checked: true; para "GUI display [default] or Terminal"}
      end
      flow(margin_top: 5) do 
        @cc = check checked: true;
        para "include C methods call" 
      end
      flow do 
        if @file == nil
          @file_button = button "choose file" do
            @file = ask_open_file 
            if @file
              @file_para.text = @file
              @trace_button.state = nil
            else
              @file_para.text = "no script, no trace !"
              @trace_button.state = "disabled"
            end
          end 
        end
      
      @trace_button = button 'Start Profile', state: (@file == nil ? "disabled": nil) do
        $shoes_profiler.add_c_calls = @cc.checked?
        Dir.chdir(File.dirname(@file)) do
          $shoes_profiler.start{ eval IO.read(File.basename(@file)).force_encoding("UTF-8"), TOPLEVEL_BINDING }

        end
      end
      @end_button = button 'End Profile' do
        nodes, links = $shoes_profiler.stop
        $shoes_profiler.nodes = nodes
        $shoes_profiler.links = links
        if @gui_display.checked? 
          visit "/graphical"
        else
          visit "/terminal"
        end
      end
    end 
    @file_para = para @file

  end
end

def filter_by(filter)
  max = $shoes_profiler.nodes.sort_by { |n,mi| n.length }[-1][0].length
  sorted = $shoes_profiler.nodes.sort_by { |n,mi| mi.send(filter) }
  usage = sorted.map {|arr| arr[1].send(filter) }
  unik = usage.uniq
        
  pre_nodes = sorted.reverse.reduce({}) do |memo,(name, method_info)|
      ca = 1.0/unik.size*(unik.index(method_info.send(filter))+1)
      memo.merge "#{name}": { node: [name, method_info], size: max, 
              color_alpha: ca, info: method_info.send(filter).to_s }
  end
  return pre_nodes
end       
    
def graphscreen # get here from a visit(url)
  stack do
    para "Results for #{$shoes_profiler.file}"
    para "Nodes: #{$shoes_profiler.nodes.length} Links: #{$shoes_profiler.links.length}"
    flow  do
      button "Counts", margin: 4 do
        @units.text = "number of times method is called"
        @result_slot.clear { filter_by(:count).each { |k,v| node_widget v } }
      end
      button "Method Time", margin: 4 do
         @units.text = "total time spent by the method alone in microseconds"
        @result_slot.clear { filter_by(:self_time).each { |k,v| node_widget v } }
      end
      button "Total Time", margin: 4 do
        @units.text = "total time spent by the method and subsequent other methods calls in microseconds"
        @result_slot.clear { filter_by(:total_time).each { |k,v| node_widget v } }
      end 
      button "Show text report", margin: 4 do 
        visit "/terminal"
      end
    end
    @units = para ""
    @result_slot = flow(margin: 5) {}
  end
end 

def textscreen # get here from a visit(url)
  stack do
    button "GUI Display" do
      visit "/graphical"
    end
    Shoes.terminal
    puts "This is not written yet. Stay tuned"
  end
end

end
Shoes.app width: 600, height: 400, resizeable: true, title: "Profiler"
