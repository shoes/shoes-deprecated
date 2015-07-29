# bug #008 edit_line finish on OSX
Shoes.app do
  stack do
    @el1 = edit_line "don't disappear"
    @el3 = edit_line ''
    @el2 = edit_line do |e|
      para e.text+"\n"
    end
    @el2.finish = proc { |slf| para "enterkey2 #{slf.text}\n" }
    button "show contents" do
      para "--------\n"
      para @el1.text
      para @el3.text
      para @el2.text
    end
  end
end
