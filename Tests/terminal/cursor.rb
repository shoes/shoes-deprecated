Shoes.app do 
  stack do
    para "Game Mode Terminal test"
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
      (3..rows+2).each {|row| puts row }
      (3..rows).each { |ln| $stdout.write "\033[#{ln};1H#{ln}"; $stdout.flush }
      $stdout.write "\033[10;10H10,10\033[20;20H20,20"
      $stdout.flush
      $stdout.write "\033[24;79HEND\033[1;1H"
      $stdout.flush
    end
  end
end
