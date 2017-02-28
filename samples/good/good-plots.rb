# good-plots.rb
Shoes.app width: 620, height: 480 do
  stack do 
    flow do
      # button "quit" do Shoes.quit end
      button "Line" do
        @plot_area.clear do 
          @values1 = [24, 22, 10, 13, 20, 8, 22]
          @x_axis1 = ['a','b','c','d','e','f', 'g']
          @values2 = [200, 150, 75, 125, 75, 225, 125]
          @x_axis2 = ['a','b','c','d','e','f', 'g']
          @grf = plot 600, 400, title: "Line Graph", caption: "Not that hard to do",
              font: "Helvetica", auto_grid: true, default: "skip", background: honeydew
          @grf.add values: @values1, labels: @x_axis1, name: "foobar", 
              min: 6, max: 26 , desc: "foobar Yy", color: dodgerblue,
              points: true
          @grf.add values: @values2, labels: @x_axis2,
              name: "Tab", min: @values2.min, max: @values2.max, desc: "BarTab", color: coral,
              points: true, strokewidth: 2
         @plot_area.refresh_slot
        end
      end
      button "Column" do
        @plot_area.clear do
          @values1 = [24, 22, 10, 13, 20, 8, 22]
          @x_axis1 = ['a','b','c','d','e','f', 'g']
          @values2 = [200, 150, 75, 125, 75, 225, 125]
          @x_axis2 = ['a','b','c','d','e','f', 'g']
          @grf = plot 600, 400, title: "Column Graph", caption: 
              "Amazing!!" , font: "Mono", auto_grid: false, 
              default: "skip", background: cornsilk, chart: "column", boundary_box: false
          @grf.add values: @values1, labels: @x_axis1,
              name: "Bar", min: 0, max:  30, desc: "foobar Yy", color: rgb(220, 20, 60, 172),
              points: true, strokewidth: 12
          cs2 = chart_series values: @values2, labels: @x_axis2,
              name: "Tab", min: 50, max: 230, desc: "BarTab", color: green,
              points: true, strokewidth: 6
         @grf.add cs2
       end
      end
      button "Scatter" do
        @plot_area.clear do
          @values1 = [14,13,15,27,17,18,33,25,21] # x values - Wind speed
          @values2 = [11,17,23,39,22,31,47,48,41] # y values - Cloud cover
          @grf = plot 600, 400, title: "Weather Conditions", caption: 
              "Scatter along, Children" , font: "Helvetica", auto_grid: true,
              default: "skip", background: honeydew, chart: "scatter"
          cs1 = @grf.add  values: @values1, 
              name: "Wind", min: 12.5, max: 35 , color: dodgerblue,
              points: "dot", strokewidth: 1, desc: "Wind speed (km/hr)"
          #cs1.points = "box"
          cs1.strokewidth = 5
          cs2 = chart_series values: @values2, name: "Clouds",
              min: 10, max: 50 , color: black,
              points: "box", strokewidth: 1, desc: "Cloud Cover (percentage)"
          @grf.add cs2
        end
      end
      button "Pie" do
        @plot_area.clear do
          @values1 = [15, 5, 4, 4, 3, 3, 1, 1] # downloads
          @obs1 = ["gtk3-32.exe", "gtk3-x86_64.install", "osx-10.9.tgz", "/",
              "3.2.25.exe", "robots.txt", "gtk3-i686,install,", "gtk3-armhf.install"]
          @grf = plot 600, 400, title: "Shoes Https downloads", caption: 
              "popularity " , font: "Helvetica", auto_grid: true,
              default: "skip", background: white, chart: "pie", pie_percent: false,
              colors: [yellow, olive]
          cs = chart_series values: @values1, labels: @obs1,
              name: "download", min: @values1.min, max: @values1.max
          @grf.add cs
        end
      end
    end
    @plot_area = stack do
    end
  end
end
