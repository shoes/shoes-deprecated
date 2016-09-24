# scatter graph - 
# data comes form OpenOffice example 
Shoes.app width: 620, height: 480 do
  @values1 = [14,13,15,27,17,18,33,25,21] # x values - Wind speed
  @values2 = [11,17,23,39,22,31,47,48,41] # y values - Cloud cover
  
  stack do
    para "Plot Scatter Demo 4"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "Weather Conditions", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: true,
          missing: "skip", background: "honeydew", chart: "scatter"
      end
    end
    @grf.add num_obs: @values1.size, values: @values1, 
      name: "Wind", minv: 12.5, maxv: 35 , color: "dodgerblue",
      nubs: "dot", strokewidth: 1
    @grf.add num_obs: @values2.size, values: @values2, name: "Clouds",
      long_name: "Cloud Cover (percentage)", minv: 10, maxv: 50 , color: "black",
      nubs: "box", strokewidth: 1

  end
end

