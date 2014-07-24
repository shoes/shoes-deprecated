Shoes.app do
  stack do
    flow {
      @eb = edit_box :width => 200, :state => "disabled";
      edit_line :width => 200, :state => "disabled"
    }  
  end
  button "Toggle EB" do
    @eb.style(:height => 50)
    # @eb.sytle(:state => nil) # doesn't work
    @eb.state = nil # works on Gtk2
  end
end
