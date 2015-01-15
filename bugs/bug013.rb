Shoes.app(:title => "Copy and Paste", :width => 450, :height => 250) {
   button "copy" do
      self.clipboard = "this is a Shoes string."
   end
   button "paste" do
      para clipboard()
   end
}
