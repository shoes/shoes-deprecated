
require 'stringio'
require 'byebug'
require 'byebug/runner'
require 'thread'

module Byebug
class MimickBye < Byebug::Interface
  # shoesobj is the Shoes.App of the Debugger screen
  def initialize shoesobj  
    super()
    @input = shoesobj
    @output = shoesobj
    @error = shoesobj
  end
  
  def read_command(prompt)
     $stderr.puts "read_command #{prompt}"
      super("#{prompt}")
  end

  def confirm(prompt)
     $stderr.puts "confirm_command #{prompt}"
      super("#{prompt}")
  end

  def close
      #output.close
  rescue IOError
      errmsg('Error closing the interface...')
  end
  
  def puts(message)
    $stderr.puts "puts: #{message}"
    @output.write_string message
  end
  
  def print(message)
    $stderr.puts "print: #{message}"
    @output.write_string message
  end

  def readline(prompt)
    $stderr.puts "waiting for signal"
    @output.write_string(prompt)
    # wait for input
    result = ''
    @input.gets_mutex.synchronize {
      @input.gets_cv.wait
      $stderr.puts "back from read_line #{result}"
      result = @input.cmd
    }
    fail IOError unless result
    result.chomp
  end
end
end

module Shoes::Debugger

  attr_accessor :gets_mutex, :gets_cv, :str, :cmd  
  def write_string  outstr
    #puts "write: #{@str} #{outstr}"
    @str += [outstr]
    @console.replace *(@str)
    @scroll.scroll_top = @scroll.scroll_max
  end
  
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
    # Now it gets tricky - need a new thread that runs byebug (and it's gets() )
    # and we want to make sure the Console window is on screen
    # and we need a mutex + condition variable
    gets_mutex = Mutex.new
    gets_cv = ConditionVariable.new
    start do # after gui is running
      byethr = Thread.new {
        Byebug.debug_load($PROGRAM_NAME, true) # this starts byebug
      }
    end
    
    @cmd = ""
    @console.cursor = -1
      keypress do |k|
       case k
       when "\n"
          @str += ["#{@cmd}\n"]
          # signal that we have a line to process.
          gets_mutex.synchronize {
            $stderr.puts "signal"
            gets_cv.signal
          }
          #write_string "#{@cmd}\n" # just echo for now
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
