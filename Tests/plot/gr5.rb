# pie graph - 
# data is Shoes https downloads on a Sunday (light day)
Shoes.app width: 620, height: 480 do
  @values1 = [15, 5, 4, 4, 3, 3, 1, 1] # downloads
  @obs1 = ["gtk3-32.exe", "gtk3-x86_64.install", "osx-10.9.tgz", "/",
     "3.2.25.exe", "robots.txt", "gtk3-i686,install,", "gtk3-armmhf.install"]
  
  stack do
    para "Plot Pie Demo 5"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "Shoes Https downloads", caption: 
          "popularilty " , font: "Helvetica", auto_grid: true,
          missing: "skip", background: "white", chart: "pie"
      end
    end
    @grf.add num_obs: @values1.size, values: @values1, xobs: @obs1,
      name: "Wind", minv: @values1.min, maxv: @values1.max, color: "dodgerblue",
      nubs: "dot", strokewidth: 1
  end
end

