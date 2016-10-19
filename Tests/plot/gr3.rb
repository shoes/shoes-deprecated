# two graphs - line and bar 
Shoes.app width: 800, height: 500 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  
#  @x_axis2 = ['a','b',nil,'d','e','f', 'g']
  @x_axis2 = ['a','b','c','d','e','f', 'g']
  @values2 = [200, 150, 75, 125, 75, 225, 125]
  # @values2 = [200, 150, 75, 125, nil, 50, 125]
  stack do
    para "Plot Demo Line and Column"
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
          default: "skip", background: "honeydew"
         @grf2 = plot widget_width, widget_height+100, title: "Column Graph", caption: 
          "Amazing!!" , font: "Mono", auto_grid: false, 
          default: "skip", background: "cornsilk", chart: "column", boundary_box: false
      end
    end
    @grf.add values: @values1, labels: @x_axis1,
      name: "foobar", min: 6, max: 26 , desc: "foobar Yy", color: "dodgerblue",
       points: true
    @grf.add values: @values2, labels: @x_axis2,
       name: "Tab", min: @values2.min, max: @values2.max, desc: "BarTab", color: "coral",
       points: true, strokewidth: 2

    @grf2.add values: @values1, labels: @x_axis1,
       name: "Bar", min: 0, max:  30, desc: "foobar Yy", color: "crimson",
       points: true, strokewidth: 12
    cs2 = chart_series values: @values2, labels: @x_axis2,
       name: "Tab", min: 50, max: 230, desc: "BarTab", color: "green",
       points: true, strokewidth: 6
    @grf2.add cs2
    #@grf2.add values: @values2, labels: @x_axis2,
    #   name: "Tab", min: 50, max: 230, desc: "BarTab", color: "green",
    #   points: true, strokewidth: 6
  end
end

