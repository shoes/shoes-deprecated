require 'readline'
Shoes.app do 
  stack do
    para "Game Mode Terminal Screen/Line test"
    button "do it" do
      columns = 80
      rows = 24
      Shoes.terminal columns: columns, rows: rows, fontsize: 12, title: "Test Cursor Movements",
        mode: 'game'
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
      puts "Line 3"
      lcnt = 1
      Thread.new do
        loop do
          $stdout.write "\033[12;40H"; $stdout.flush
          ans = Readline.readline("Erase Line 2 [y/n/q] ?").strip
          break if ans == 'n' 
          exit if ans == 'q'
          if ans == 'y'
            $stdout.write "\033[2;1H\033[2K\033[2;1HHere"; $stdout.flush
          end
          $stdout.write "\033[12;40H"; $stdout.flush
          ans = Readline.readline("Erase Line 1 Left half [y/n/q] ?").strip
          break if ans == 'n' 
          exit if ans == 'q'
          if ans == 'y'
            $stdout.write "\033[1;40H\033[1K\033[2;1HDone"; $stdout.flush
          end
          $stdout.write "\033[12;40H"; $stdout.flush
          ans = Readline.readline("Erase Line 1 Right half [y/n/q] ?").strip
          break if ans == 'n' 
          exit if ans == 'q'
          if ans == 'y'
            $stdout.write "\033[1;40H\033[K\033[2;1HAll Finished"; $stdout.flush
          end
        end
        puts "test finished"
      end
    end
  end
end
