# mix of problems with testing framework, cshoes, #174, #224
Shoes.app do
  @stk = stack do
    button "quit" do Shoes.quit end
  end
  @stk.start do
  #timer(0.1) do
    para "In start blocks"
    #Shoes.show_console
    $stdout = $stderr
    $stdout.write "stdout printing\n"
    $stderr.puts "stderr printing"
    #if $stdout != $stderr
    #  $stdout = $stderr
      puts "Now on stdout?"
    #end
  end
end
