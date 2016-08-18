# expert-graph.rb? draw timeseries data - using Shoes::Widget
class Series
  attr_accessor :size, :minv, :maxv, :ary
  
  def initialize(opts = {})
    if opts[:values]
     @ary = opts[:values]
     @size = @ary.size
    else
      raise "must specifiy an :values array"
    end
    @size = opts[:num_obs] ? opts[:num_obs] : @ary.size
    @maxv = opts[:maxv] ? opts[:maxv] : @ary.max
    @minv = opts[:minv] ? opts[:minv] : @ary.min
  end
  
  def [] (idx)
    return @ary[idx]
  end
  
end
class Graph < Shoes::Widget
  @@series_collection = Array.new
  attr_accessor :width, :height, :canvas, :series_collection

  def initialize(opts = {})
    @widget_w = opts[:width] || 200
    @widget_h = opts[:height] || 200
    self.width = @widget_w
    self.height = @widget_h
    @canvas = flow height: @widget_h, width: @widget_w  do
      background white
    end
  end

  
  def draw_all_series
    @@series_collection.each do |ser| 
      @canvas.stroke blue
      #assume top left is 0,0 bottom right is 800,500
      wid = @widget_w
      hgt = @widget_h
      vscale = hgt / (ser.maxv - ser.minv)
      hscale = wid / ser.size
      oldx = 0
      oldy = 0
      (0..ser.size-1).each do |i|
        v = ser[i]
        x = (i * hscale).round.to_i
        y = (hgt - ((v - ser.minv) * vscale).round).to_i
        if i == 0
          @canvas.line(x, y, x, y)
        else
          @canvas.line(oldx, oldy, x, y)
        end
        oldx = x
        oldy = y
      end
    end
  end
  
  def add_series(series)
    @@series_collection << series
    draw_all_series
  end
  
  def remove_series(series)
  end
end

Shoes.app width: 620, height: 610 do
  @values1 = [24, 22, 10, 15, 12, 8, 22]
  @x_axis1 = ['a','b','c','d','e','f','g']
  stack do
    para "Just Art Graph Demo"
    widget_width = 600
    widget_height = 400
    @grf = graph width: widget_width, height: widget_height
    tseries = Series.new num_obs: @values1.size, values: @values1, xobs: @x_axis1,
       name: "foobar", minv: 6, maxv: 26 , long_name: "foobar values"
    @grf.add_series(tseries)
    flow do 
      button "quit" do Shoes.quit end
      button "redraw" do
        @grf.draw_series(tseries)
      end
    end
  end
end

