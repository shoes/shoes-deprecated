Shoes.app do 
  stack do
    para "Terminal test"
    button "do it" do
      #Shoes.show_console
      columns = 80
      Shoes.terminal columns: columns, rows: 24, fontsize: 12, title: "Bug236",
        fg: "yellow", bg: "black"
      #if RUBY_PLATFORM =~ /darwin/
      #  $stderr.puts "Filenums #{STDOUT.fileno} #{STDERR.fileno}"
      #  $stdout = $stderr
      #end
      $stderr.puts "STDERR OK"
      $stdout.puts "STDOUT OK" 
      puts "Way to go!"
      puts "\033[32mGood in green?\033[00m or is \033\[35mthis better\033\[00m"
      puts "\033[31m\033[40mRed on Black\033[0m OK? \033[01mBold?\033[0m"
      puts "And \033[04;33;46mUnderline joy?\033\[0m"
      ruler1 = ''
      ruler2 = ''
      (1..columns).each do |i|
        tens = i/10
        ones = i%10
        ruler1 << (tens == 0 ? ' ' : tens.to_s)
        ruler2 << ones.to_s
      end
      puts ruler1
      puts ruler2
    end
  end
end
