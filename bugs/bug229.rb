# OSX bug edit_line vertical alignment in flow. edit_box is OK.
Shoes.app do
    flow {
      edit_line :width => 200, :state => "disabled";
      edit_line :width => 200, :state => "disabled"
    } 
  end
end
