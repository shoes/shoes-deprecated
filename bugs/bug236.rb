Shoes.app do 
  stack do
    para "Console test"
    button "do it" do
      Shoes.show_console
      #if RUBY_PLATFORM =~ /darwin/
      #  $stderr.puts "Filenums #{STDOUT.fileno} #{STDERR.fileno}"
      #  $stdout = $stderr
      #end
      $stderr.puts "STDERR OK"
      $stdout.puts "STDOUT OK" 
      puts "Way to go!"
      puts "\033[32mGood in green?\033[00m or is \033\[35mthis better\033\[00m"
      puts "\033[31m\033[40mRed on Black\033[0m OK? \033[01mBold?\033[0m"
      puts "And \033[04;33mUnderline joy?\033\[0m"
      $stderr.puts "We can perform some cursor test - enter 'q' to quit, 'n' to skip"
      require 'readline'
      Thread.new do
        loop do
          ln = Readline.readline('Clear screen? ', false)
          ans = ln.strip
          exit if ans == 'q'
          puts 
        end
      end
    end
  end
end
