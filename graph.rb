# expert-graph.rb? draw timeseries data - using Shoes::Widget
require 'lib/shoes/dataseries/csvseries.rb'
tseries = CsvSeries.create('Tests/tstest.csv') 
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
      hscale = wid / ser.length
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
  stack do
    tsname = tseries.name
    tstype = tseries.bool?
    tsminv = tseries.minv
    tsmaxv = tseries.maxv
    tsbeg = tseries.start_date # datetime
    tsend = tseries.end_date    # datetime

    para "have  #{tsname}  boolean?: #{tstype}  vrange: #{tsminv} to #{tsmaxv}"
    para "First [0] #{tseries.value_at_index(0)}"
    para "Last  [#{tseries.size-1}] #{tseries.value_at_index(tseries.size-1)}"
    para "start #{tseries.value_at_date(tseries.start_date)} #{tseries.start_date} "
    para "end   #{tseries.value_at_date(tseries.end_date)} #{tseries.end_date}"
    rdidx = rand(tseries.length);
    v = tseries[rdidx]
    #para "random: #{rdidx}: is  #{v}"
    widget_width = 600
    widget_height = 400
    #@grf = graph width: widget_width, height: widget_height
    #@grf.add_series(tseries)
    @grf = plot widget_width, widget_height
    @grf.add tseries 
    flow do 
      button "quit" do Shoes.quit end
      button "redraw" do
      end
    end
  end
end

