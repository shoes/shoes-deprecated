# good-graph.rb ?
Shoes.app width: 700, height: 610 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  #@values1 = [24, 22, nil, 13, 20, 8, 22]
  #@x_axis1 = ['a','b',nil,'d','e','f', 'g']
  @values2 = [200, 150, 75, 125, 75, 50, 125]
  stack do
    para "Plot Widget Demo"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        para "This is mine!"
        @grf = plot widget_width, widget_height, title: "My Graph", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: true, chart: "timeseries",
          default: "skip", click: proc {|btn, l, t| puts "click on #{@grf.near_x(l)}" }
      end
    end
    @grf.add  values: @values1, labels: @x_axis1,
       name: "foobar", min: 6, max: 26 , desc: "foobar Yy", color: "dodgerblue",
       points: true
    flow do 
      button "add #2" do
        @grf.add values: @values2,
          name: "bartab", min: @values2.min, max: @values2.max , color: "coral",
          strokewidth: 3 #, points: true
      end
      button "delete #2" do
        @grf.delete(1)
      end
      button "drop two right" do
        last = @grf.last
        #puts "chopping #{last} bye two"
        @grf.set_last last-2
      end
      button "drop one left" do
        @grf.set_first (@grf.first + 1)
      end
      button "save as" do
        file = ask_save_file
        @grf.save_as file if file
      end
    end
    button "Add points to #1" do
      s2 = @grf.id("bartab")
      @grf.delete(s2) if s2  # make sure only series1 is on screen
      idx = @values1.size
      m =  @values1.min      # min and max don't work if nil is in the data
      rng = @values1.max - m
      i = 0
      tmr = every(1) do
        tmr.stop if i >= 5 
        rv = rand(rng) + m
        idx = idx + 1
        @values1 << rv
        @x_axis1 << idx.to_s
        @grf.redraw_to(idx) #problematic
        i = i + 1
      end
    end
  end
  def grf_clicked (me, btn, t, l)
    puts "clicked #{btn}, #{t}, #{l}"
  end
end

