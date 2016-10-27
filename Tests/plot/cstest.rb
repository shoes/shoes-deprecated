# chart_series test 
Shoes.app width: 600, height: 500 do
  @values1 = [24, 22, 10, 13, 20, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f', 'g']
  @top = stack do
    para "Chart series tests"
    flow do 
      button "quit" do Shoes.quit end
      button "List" do 
        @top.append do
         v = @cs.values
         l = @cs.labels
         para "Length = #{v.size} min: #{@cs.min}, max: #{@cs.max}"
         para "short: #{@cs.name}, desc: #{@cs.desc} color: #{@cs.color}"
         para "stoke: #{@cs.strokewidth} points: #{@cs.points}"
         v.each_index do |idx|
           para "#{l[idx]} #{v[idx]}"
         end
        end
      end
      button "Change Things" do
        @cs.min = 4
        @cs.max = 30
        @cs.desc = "Nothing Like It"
        @cs.color = coral
        @cs.strokewidth = 2
        @cs.points = "circle"  # line charts do respect this, scatter does (gr4.rb)
        ary = @cs.get(3)
        puts "get x: #{ary[0]} y: #{ary[1]}"
        @cs.set 7, ['whoa!', 29]
        ary = @cs.get(7)
        puts "set x: #{ary[0]} y: #{ary[1]}"        
        # trigger redraw of chart
        @grf.redraw_to(@values1.size)
      end
    end
    widget_width = 580
    widget_height = 300
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "My Graph", caption: 
          "Look at that! Booyah!!" , font: "Helvetica", auto_grid: true,
          default: "skip", background: honeydew
      end
    end
    @cs = @grf.add values: @values1, labels: @x_axis1,
      name: "foobar", min: 6, max: 26 , desc: "foobar Yy", color: dodgerblue,
       points: true
  end
end

