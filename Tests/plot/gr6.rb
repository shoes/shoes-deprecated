# Radar graph - 
# data is Shoes https downloads on a Sunday (light day)
Shoes.app width: 620, height: 480 do
  @xobs = ["internet", "television", "radio", "newspaper", "magazine"]
  @values1 = [80,        160,          145,     75,          80] # in k$
  @values2 = [180,        90,           95,     90,          90]
  
  stack do
    para "Plot Radar Demo 5"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "Advertising", caption: 
          "Budget Spend" , font: "Helvetica", auto_grid: true,
          missing: "skip", background: "white", chart: "radar", pie_percent: false
      end
    end
    @grf.add num_obs: @values1.size, values: @values1, xobs: @xobs,
      name: "Year 1", minv: 0, maxv: 200, color: "dodgerblue",
      nubs: "dot", strokewidth: 1
    #@grf.add num_obs: @values2.size, values: @values2, xobs: @xobs,
    #  name: "Year 2", minv: 0, maxv: 200, color: "coral",
    #  nubs: "dot", strokewidth: 1
  end
end

