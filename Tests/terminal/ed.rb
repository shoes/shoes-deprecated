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
          $stdout.write "\033[24;78HEND"
          $stdout.write "\033[12;40H";   $stdout.flush
          ans = Readline.readline("Erase Display ?").strip
          exit if ans == 'q'
          if ans != ''
            $stdout.write "\033[2;1H\033[2J\033[2;1HDONE"; $stdout.flush
          end
          $stdout.write "\033[1;1H\033[32m"; $stdout.flush
          ans = Readline.readline("next [rtn/q] ?").strip
          exit if ans == 'q'
          
          $stdout.write "\033[20;1H"
          puts ruler1
          puts ruler2
          $stdout.write "\033[24;78HEND"
          $stdout.write "\033[12;40H";   $stdout.flush
          ans = Readline.readline("Erase Display Below [y/q] ?").strip
          if ans == 'y'
            $stdout.write "\033[J"; $stdout.flush
          end
          $stdout.write "\033[1;1H\033[31m"; $stdout.flush
          ans = Readline.readline("continue [rtn/q] ?").strip
          exit if ans == 'q'
          
          $stdout.write "\033[1;1H"
          puts ruler1
          puts ruler2
          $stdout.write "\033[12;72HHERE"
          $stdout.write "\033[12;40H";   $stdout.flush
          ans = Readline.readline("Erase Display to HERE [y/q] ?").strip
          if ans == 'y'
            $stdout.write "\033[12;72H\033[1J"; $stdout.flush
          end
          $stdout.write "\033[24;1H"
          ans = Readline.readline("nothing more to do - [rtn] ").strip
          quit
        end
        puts "test finished"
      end
    end
  end
end
