# Radar graph - 
Shoes.app width: 620, height: 480 do
  @xobs = ["internet", "television", "radio", "newspaper", "magazine"]
  @values1 = [80,        160,          145,     75,          80] # in k$
  @values2 = [180,        90,           95,     90,          90]
  
  stack do
    para "Plot Radar Demo 6"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "Advertising", caption: 
          "Budget Spend" , font: "Helvetica", auto_grid: true,
          default: "skip", background: white, chart: "radar"
      end
    end
    @grf.add values: @values1, labels: @xobs,
      name: "Year 1", min: 0, max: 200, color: dodgerblue,
      points: "dot", strokewidth: 1
    cs = app.chart_series values: @values2, labels: @xobs,
      name: "Year 2", min: 0, max: 200, color: coral,
      points: "dot", strokewidth: 1
    @grf.add cs
  end
end

