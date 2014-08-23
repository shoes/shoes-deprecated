Shoes.app do
  stack do
    @el = edit_line do |e|
      para e.text+"\n"
    end
    @el.finish = proc { |slf| para "enterkey #{slf.text}\n" }
  end
end
