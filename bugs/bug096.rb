Shoes.app do
  stack do
    para "Display some croation characters\n"
    title "\u0161 \u0111 \u010d \u0107 \u017e"
    edit_line do |c|
      @p.text = c.text
    end
   @p = para ""
  end
end
