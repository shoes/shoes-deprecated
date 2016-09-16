# two grapsh
Shoes.app width: 800, height: 500 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  
  @x_axis2 = ['a','b',nil,'d','e','f', 'g']
  #@values2 = [200, 150, 75, 125, 75, 50, 125]
  @values2 = [200, 150, 75, 125, nil, 50, 125]
  stack do
    para "Plot Widget Demo 2"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 400
    widget_height = 300
    stack do
      flow do
        #para "This is mine!"
        @grf = plot widget_width, widget_height, title: "My Graph", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: true,
          missing: "skip"
         @grf2 = plot widget_width, widget_height+100, title: "2nd Graph", caption: 
          "Amazing!!" , font: "Helvetica", auto_grid: false, chart: "line",
          missing: "skip", background: "cornsilk"
      
      end
    end
    @grf.add num_obs: @values1.size, values: @values1, xobs: @x_axis1,
       name: "foobar", minv: 6, maxv: 26 , long_name: "foobar Yy", color: "dodgerblue",
       nubs: true
    @grf2.add num_obs: @values2.size, values: @values2, xobs: @x_axis2,
       name: "Bar", minv: 50, maxv:  250, long_name: "BarBaz", color: "dodgerblue",
       nubs: true
  end
end

