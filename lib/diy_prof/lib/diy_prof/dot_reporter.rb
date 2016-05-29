module DiyProf

  CallInfo = Struct.new(:name, :time)
  MethodInfo = Struct.new(:count, :total_time, :self_time)

  class DotReporter
    def initialize
      # A stack for pushing/popping methods when methods get called/returned
      @call_stack = []
      # Holds nodes
      @methods = {}
      # Holds connections among nodes
      @calls = {}
    end

    def record(event, method_name, time)
      case event
      when :call
        @call_stack << CallInfo.new(method_name, time)
      when :return
        # Return cannot be the first event in the call stack
        return if @call_stack.empty?

        method = @call_stack.pop
        # Set execution time of method in call info
        method.time = time - method.time
        add_method_to_call_tree(method)
      end
    end

    def result
      dot_notation
    end

    private

    def add_method_to_call_tree(method)
      # Add method as a node to the call graph
      @methods[method.name] ||= MethodInfo.new(0, 0, 0)
      # Update total time spent inside the method
      @methods[method.name].total_time += method.time
      @methods[method.name].self_time += method.time
      # Update total no of times the method was called
      @methods[method.name].count += 1

      # If the method has a parent in the call stack
      # Add a connection from the parent node to this method
      if parent = @call_stack.last
        @calls[parent.name] ||= {}
        @calls[parent.name][method.name] ||= 0
        @calls[parent.name][method.name] += 1

        # Take away self_time of parent
        @methods[parent.name] ||= MethodInfo.new(0, 0, 0)
        @methods[parent.name].self_time -= method.time
      end
    end

    def dot_notation
      dot = %Q(
        digraph G {
          #{graph_nodes}
          #{graph_links}
        }
      )
    end

    def graph_nodes
      nodes = ""
      @methods.each do |name, method_info|
        nodes << "#{name} [label=\"#{name}\\ncalls: #{method_info.count}\\ntotal time: #{method_info.total_time}\\nself time: #{method_info.self_time}\"];\n"
      end
      nodes
    end

    def graph_links
      links = ""
      @calls.each do |parent, children|
        children.each do |child, count|
          links << "#{parent} -> #{child} [label=\" #{count}\"];\n"
        end
      end
      links
    end
  end
end
