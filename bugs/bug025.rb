Shoes.app(:title => "Shoes Events", :width => 450, :height => 250) {
   start { |n| para "starting #{n}\n" }
   stack { finish { |n| alert "finishing #{n}" } }
   window(:title => "Window Events", :resizable => false) {
     stack do
      finish { |n| alert "finishing #{n}" }
     end
   }
}
