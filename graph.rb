# good-graph.rb
Shoes.app width: 620, height: 610 do
  @values1 = [24, 22, 10, 15, 12, 8]
  @x_axis1 = ['a','b','c','d','e','f']
  stack do
    para "Plot Widget Demo"
    flow do 
      button "quit" do Shoes.quit end
      button "redraw" do
      end
    end
    widget_width = 600
    widget_height = 400
    @grf = plot widget_width, widget_height, title:"My Graph", caption: 
      "look at that!"
    @grf.add num_obs: @values1.size, values: @values1, #xobs: @x_axis1,
       name: "foobar", minv: 6, maxv: 26 , long_name: "foobar values"
  end
end

