Shoes.app :width => 800 do
    para "First"
    list_box :width => 100
    @e1 = edit_line :width => 100, :state => "ensabled"
    para "Two"
    list_box :width => 100
    @e2 =edit_line :width => 100, :state => "disabled"
    para "Three"
    @e3 = edit_box :width => 100, :state => "disabled"
    para "Four"
    @e4 = edit_line width: 100, :state => "disabled"
    para "Five"
end

