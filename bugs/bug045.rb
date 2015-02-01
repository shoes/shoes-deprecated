Shoes.app do
   @l = list_box :items => ["first", "second"], :choose => "first"

   @e = edit_line

   button "ok" do
      @l.items << @e.text
   end
end
