# good-graph.rb
Shoes.app width: 620, height: 610 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  @values2 = [200, 150, 75, 125, 75, 100, 125]
  stack do
    para "Plot Widget Demo"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    @grf = plot widget_width, widget_height, title:"My Graph", caption: 
      "look at that!"
    @grf.add num_obs: @values1.size, values: @values1, xobs: @x_axis1,
       name: "foobar", minv: 6, maxv: 26 , long_name: "foobar values"
    button "add #2" do
      @grf.add num_obs: @values2.size, values: @values2,
        name: "bartab", minv: @values2.min, maxv: @values2.max
    end
  end
end

