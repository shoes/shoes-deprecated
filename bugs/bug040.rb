Shoes.app do
  @a = edit_box :height => 100
  str = ""
  10.times {|i| str << "Line #{i}\n"}
  @a.text = str
  button "Display" do
   para @a.text
   @a.text = @a.text + "Some stuff at the end\n" 
  end
end
