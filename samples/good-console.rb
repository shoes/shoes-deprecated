Shoes.app do
  stack do
    para "This will show the console and run a loop that echos keyboard \
input until you enter 'quit' at the beginning of a line. quit will exit Shoes \n"
   button "run loop" do
      Shoes.terminal
      require 'readline'
      Thread.new do
        loop do
          ln = Readline.readline('> ', false)
          if ln.strip == 'quit'
            $stderr.write "really quit (y/n) "
            ans = Readline.readline.strip
            Shoes.exit if ans == 'y'
          end
          $stdout.puts "RL: #{ln}"
        end
      end
    end
  end
end
