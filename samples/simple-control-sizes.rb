Shoes.app :width => 360, :height => 600, :resizable => false do
  stroke "#dde"
  background "#f1f5ff"
  12.times { |x| line 20, 142 + (30 * x), 320, 142 + (30 * x) }

  stack :margin => 20 do
    title "Control Sizes", :size => 16
    para "This app measures various controls against a grid of lines, to be sure they size appropriately despite the platform."
    stack :margin_left => 20 do
      button "Standard"
      button "Margin: 2, Height: 28", :margin => 2, :height => 30
      edit_line "Standard", :margin => 1
      edit_line "Margin: 4, Height: 30", :height => 30, :margin => 4
      list_box :items => ["Standard"], :choose => "Standard"
      list_box :items => ["Margin: 4, Height: 32"], 
        :choose => "Margin: 4, Height: 32",
        :height => 32, :margin => 4
      progress
      progress :height => 32, :margin => 4
      edit_box
    end
  end
end
