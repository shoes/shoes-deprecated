Shoes.app :width => 400, :height => 100 do
 stack do
   items = []
   items[0]= "No! Don't tell me"
   items[1]= "Let me tell you a story about how octal numbers fell out of \
favor and were replaced with hexidecimal numbers. As you might suspect the \
answer is not technical but financial and related to the 80 column punched card"
   list_box :items => items, :choose => items[1]
  end
end
