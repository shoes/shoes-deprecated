# scatter graph - line without the lines? 
# set min and max yourself helps produce something meaning ful
Shoes.app width: 420, height: 420 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  
#  @x_axis2 = ['a','b',nil,'d','e','f', 'g']
  @x_axis2 = ['a','b','c','d','e','f', 'g']
  @values2 = [200, 150, 75, 125, 75, 225, 125]
  # @values2 = [200, 150, 75, 125, nil, 50, 125]
  stack do
    para "Plot Scatter Demo 4"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 400
    widget_height = 300
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "My Graph", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: false,
          missing: "skip", background: "honeydew", chart: "scatter"
      end
    end
    @grf.add num_obs: @values1.size, values: @values1, xobs: @x_axis1,
      name: "foobar", minv: 6, maxv: 26 , long_name: "foobar Yy", color: "dodgerblue",
       nubs: true
    @grf.add num_obs: @values2.size, values: @values2, xobs: @x_axis2,
       name: "Tab", minv:50, maxv: 300, long_name: "BarTab", color: "coral",
       nubs: true, strokewidth: 2
  end
end

