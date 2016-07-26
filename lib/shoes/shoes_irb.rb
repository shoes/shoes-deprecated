require 'irb/ruby-lex'
require 'stringio'

class MimickIRB < RubyLex
  attr_accessor :started, :history, :echo

  class Continue < StandardError; end
  class Empty < StandardError; end

  def initialize
    super
    $stdout = StringIO.new
    set_input(StringIO.new)
    @started = Time.now
    @history = []
    @echo = true
  end

  def run(str)
    obj = nil
    @io << str
    @io.rewind
    unless l = lex
      raise Empty if @line == ''
    else
      case l.strip
      when "reset"
        @line = ""
      when "time"
        @line = "puts %{This IRB session started #{Time.at(Time.now.getutc - @started.getutc.to_f).strftime('%H:%M:%S')} ago.}"
      when /^echo( (.*))?/
        case $2
        when "on"
           @echo = true
           @line = "puts %{echo is toggled on}"
        when "off"
           @echo = false
           @line = "puts %{echo is toggled off}"
        else
           @line = "puts %{echo is toggled #{@echo ? "on" : "off"}}"
        end
      when "history"
        @line = "puts %{#{@history.join("\n")}}"
      when "save"
        filename = ask_save_file
        if filename
           File.open(filename, "w") { |f| f.write(@history.join("\n")) }
           @line = "puts %{Command history saved to #{filename.inspect}...}"
        else
          @line = ""
        end
      when "help"
        message = <<-END.gsub(/^ {12}/, '')
            help\tdisplay this help.
            echo\techo, echo on and echo off to toggle object output. 
            time\tdisplay time since the IRB session started.
            history\tlist command history.
            save\tsave command history to a file.
            reset\treset terminal.
        END
        @line = "puts \"#{message}\""
      else
        @line << l << "\n"
        if @ltype or @continue or @indent > 0
          raise Continue
        end
      end
    end
    unless @line.empty?
      obj = eval @line, TOPLEVEL_BINDING, "(irb)", @line_no
    end
    @line_no += @line.scan(/\n/).length
    @line = ''
    @exp_line_no = @line_no

    @indent = 0
    @indent_stack = []

    $stdout.rewind
    output = $stdout.read
    $stdout.truncate(0)
    $stdout.rewind
    [output, obj]
  rescue Object => e
    case e when Empty, Continue
    else @line = ""
    end
    raise e
  ensure
    set_input(StringIO.new)
  end

end

CURSOR = ">>"

def Shoes.shoes_irb
   Shoes.app do
     irbalike = MimickIRB.new
     @history = { :cmd => irbalike.history, :pointer => 0 }
     @str, @cmd = [CURSOR + " "], ""
     stack :width => 1.0, :height => 1.0 do
       background "#555"
       stack :width => 1.0, :height => 30 do
         background white
         para "Interactive Ruby ready.", :stroke => red
       end
       @scroll =
         stack :width => 1.0, :height => -50, :scroll => true do
           background "#555"
           @console = para @str, :font => "Monospace 12px", :stroke => "#dfa", :wrap => "char"
           @console.cursor = -1
         end
     end
     keypress do |k|
       case k
       when "\n"
         begin
           @history[:cmd] << @cmd
           @history[:pointer] = @history[:cmd].size
           out, obj = irbalike.run(@cmd)
           @str += ["#@cmd\n",
             irbalike.echo ? 
               span("#{out}=> #{obj.inspect}\n", :stroke => "#fda") :
               span("#{out}", :stroke => "#fda"),
             "#{CURSOR} "]
           @cmd = ""
           @console.cursor = -1
         rescue MimickIRB::Empty
         rescue MimickIRB::Continue
           @str += ["#@cmd\n.. "]
           @cmd = ""
         rescue Object => e
           @str += ["#@cmd\n", span("#{e.class}: #{e.message}\n", :stroke => "#fcf"),
             "#{CURSOR} "]
           @cmd = ""
         end
       when String
         @cmd.insert(@console.cursor, k)
       when :backspace
         @cmd.slice!(@console.cursor)
       when :delete
         @cmd.slice!(@console.cursor += 1) if @console.cursor < -1
       when :tab
         @cmd += "  "
       when :alt_q
         quit
       when :alt_c
         self.clipboard = @cmd
       when :alt_v
         @cmd += self.clipboard
       when :left
         @console.cursor -= 1 unless @cmd.length < -@console.cursor
       when :right
         @console.cursor += 1 if @console.cursor < -1
       when :up
         if @history[:pointer] > 0
            @history[:pointer] -= 1
            @cmd = @history[:cmd][@history[:pointer]].dup
         end
       when :down
         if @history[:pointer] < @history[:cmd].size
            @history[:pointer] += 1
            @cmd = @history[:cmd][@history[:pointer]].dup
         end
       end
       @console.replace *(@str + [@cmd])
       @scroll.scroll_top = @scroll.scroll_max
     end
   end
end
