Shoes.app do
  Shoes::show_console
  para "#{Time.now}\n"    
  para "#{Time.local(2015, 06, 19)}\n"
  $stdout.puts "do you see this?"
  require 'readline'
  require 'io/console'
  Thread.new do
    loop do
     #$stdout.write "prompt: "
     #ln = $stdin.gets
     #ln = $stdin.readline
     #Readline::vi_editing_mode
     ln = Readline::readline('> ', false)
     #ln = STDIN.cooked(&:gets)

      if ln.strip == 'quit' 
        $stderr.write "really quit (y/n)"
        ans = $stdin.gets.strip
        Shoes.quit if ans == 'y'
      end
      $stdout.puts "Shoes: #{ln}"
    end
  end
end
