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
          default: "skip", background: "honeydew", chart: "scatter"
      end
    end
    @grf.add  values: @values1, 
      name: "Wind", min: 12.5, max: 35 , color: "dodgerblue",
      points: "dot", strokewidth: 1
    cs = app.chart_series values: @values2, name: "Clouds",
      min: 10, max: 50 , color: "black",
      points: "box", strokewidth: 1, desc: "Cloud Cover (percentage)"
    @grf.add cs
    #@grf.add  values: @values2, name: "Clouds",
    #  desc: "Cloud Cover (percentage)", min: 10, max: 50 , color: "black",
    #  points: "box", strokewidth: 1
  end
end

