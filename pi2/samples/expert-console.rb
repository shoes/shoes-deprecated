 def getln_raw ()
    ln = ""
    begin
    loop do
      ch = $stdin.getch
      if ch == ?\b # backspace
        len = ln.length
        if len > 0
          $stdout.write "\b \b"
          ln.chop!
        end
      elsif ch == ?\r
        $stdout.write("\r\n")
        return ln
      else
        $stdout.putc ch
        ln += ch
      end
    end
    rescue => e
      $stdout.puts e.inspect
    end
 end

Shoes.app do
  stack do
    para "This will show the console and run a loop that echos keyboard \
input until you enter 'quit' at the beginning of a line. quit will exit Shoes \n"
    para "Select your input method"
    flow { @r1 = radio :io; para "io/console raw (Not for OSX !)"}
    flow { @r2 = radio :io; para "io/console cooked (Not for OSX !)"}
    flow { @r3 = radio :io; para "readline (all platforms)" }
    button "run loop" do
      Shoes::show_console
      if @r1.checked? && RUBY_PLATFORM =~ /linux/
        require 'io/console'
        STDIN.raw!
        Thread.new do
          $stdout.write "> "
          loop do
            ln = getln_raw
            if ln.strip == 'quit'
              $stderr.write "really quit (y/n)"
              ans = getln_raw.strip
              exit if ans == 'y'
            end
            $stdout.puts "IR: #{ln}"
          end
        end
      elsif @r2.checked? && RUBY_PLATFORM =~ /linux/
        require 'io/console'
        STDIN.cooked!
        Thread.new do
          $stdout.write "> "
          loop do
            ln = STDIN.cooked(&:gets)
            if ln.strip == 'quit'
              $stderr.write "really quit (y/n)"
              ans = $stdin.gets.strip
              exit if ans == 'y'
            end
            $stdout.puts "IC: #{ln}"
          end
        end
      elsif @r3.checked?
        require 'readline'
        Thread.new do
          loop do
            ln = Readline.readline('> ', false)
            if ln.strip == 'quit'
              $stderr.write "really quit (y/n)"
              ans = Readline.readline.strip
              exit if ans == 'y'
            end
            $stdout.puts "RL: #{ln}"
          end
        end
      end
    end
  end
end
