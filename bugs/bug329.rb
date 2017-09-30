Shoes.app do
   stack do
      @p = para
   end
   
   stack(left: self.width / 4, top: 10) do
      button("hello")
      flow { check; para "check?" }
      edit_box
      edit_line
      list_box :items => ["one", "two", "three"]
      progress
      stack do
          radio; para strong("one")
          radio; para strong("two")
      end
      slider
      spinner
      switch
   end

   click do |btn, left, top|
      @p.text = "click: #{btn}, #{left}, #{top}\n"
   end
end
