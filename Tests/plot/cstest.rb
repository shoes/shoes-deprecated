# chart_series test 
Shoes.app width: 600, height: 500 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  @top = stack do
    para "Chart series tests"
    flow do 
      button "quit" do Shoes.quit end
      button "basics" do 
        @top.append do
         v = @cs.values
         l = @cs.labels
         para "Length = #{v.size} min: #{@cs.min}, max: #{@cs.max}"
         para  "short: #{@cs.name}, desc: #{@cs.desc}"
         v.each_index do |idx|
           para "#{l[idx]} #{v[idx]}"
         end
        end
      end
      button "mix/max" do
        @cs.min = 4
        @cs.max = 30
        @cs.desc = "Nothing Like It"
        @grf.redraw_to(7)
      end
    end
    widget_width = 480
    widget_height = 300
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "My Graph", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: true,
          default: "skip", background: "honeydew"
      end
    end
    @cs = @grf.add values: @values1, labels: @x_axis1,
      name: "foobar", min: 6, max: 26 , desc: "foobar Yy", color: "dodgerblue",
       points: true
  end
end

