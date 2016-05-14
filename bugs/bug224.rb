Shoes.app do 
  stack do
    para "Terminal test"
    button "do it" do
      columns = 80
      Shoes.show_console
      if RUBY_PLATFORM =~ /darwin/
        $stdout = IO.new(1, 'w')
        # flush anything terminal C code might have written
        $stderr.flush
        $stdout.flush
        #class IO
        #  def puts args
        #    super args
        #    self.flush if self.fileno < 3
        #  end
        #end
      end
      $stderr.puts "STDERR OK"; 
      $stdout.puts "STDOUT OK"; 
      puts "Way to go!"
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
