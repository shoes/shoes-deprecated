
require 'stringio'
require 'byebug'
require 'byebug/runner'

module Byebug
class MimickBye < Byebug::Interface
  def initialize shoesin_cb
    super()
    input = StringIO.new # just for testing
    output = StringIO.new
    error = @output
  end
  
  def read_command(prompt)
     puts "read_command #{prompt}"
      super("PROMPT #{prompt}")
  end

  def confirm(prompt)
     puts "confirm_command #{prompt}"
      super("CONFIRM #{prompt}")
  end

  def close
      output.close
  rescue IOError
      errmsg('Error closing the interface...')
  end

  def readline(prompt)
     puts "readline #{prompt}"
      output.puts(prompt)

      result = shoesin_cb  # hook to 
      fail IOError unless result

      result.chomp
  end
end
end

module Shoes::Debugger

  def setup path
    $stdout.puts "Debugger setup called"
    # Byebug::Runner.new.run
    dbg = Byebug::Runner.new
    $PROGRAM_NAME = path
    # patch in IO
    intf = Byebug::MimickBye.new self
    Byebug.handler.interface = intf # expect trouble
    @str, @cmd = [], ""
    stack :width => 1.0, :height => 1.0 do
      stack :width => 1.0, :height => 30 do
        background white
        para "Shoes debugger ready.", :stroke => red
      end
      @scroll =
        stack :width => 1.0, :height => -50, :scroll => true do
          @console = para @str, :font => "Monospace 14px", :wrap => "char"
          @console.cursor = -1
        end
    end
    Byebug.debug_load($PROGRAM_NAME, true) # works if handler defaults.
  end
  
  def write_string  outstr
    #puts "write: #{@str} #{outstr}"
    @str += [outstr]
    @console.replace *(@str)
    @scroll.scroll_top = @scroll.scroll_max
  end
  
  # keychars assembled into string and passed to blk when return/enter is
  # typed. String will include '\n' so it behaves like gets()  
  def read_line &blk
    @cmd = ""
    @console.cursor = -1
      keypress do |k|
       case k
       when "\n"
          @str += ["#{@cmd}\n"]
          blk.call "#{@cmd}\n"
          @cmd = ""
          @console.cursor = -1
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
