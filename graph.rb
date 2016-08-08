# draw timeseries data - throwaway code 
require 'lib/shoes/dataseries/csvseries.rb'
tseries = CsvSeries.create('Tests/tstest.csv') 
Shoes.app width: 620, height: 610 do
  stack do
    tsname = tseries.name
    tstype = tseries.bool?
    tsminv = tseries.minv
    tsmaxv = tseries.maxv
    tsbeg = tseries.start_date # datetime
    tsend = tseries.end_date    # datetime

    para "have  #{tsname} boolean?: #{tstype} vrange: #{tsminv} to #{tsmaxv}"
    para "First [0] #{tseries.value_at_index(0)}"
    para "Last  [#{tseries.size-1}] #{tseries.value_at_index(tseries.size-1)}"
    para "start #{tseries.value_at_date(tseries.start_date)} #{tseries.start_date} "
    para "end   #{tseries.value_at_date(tseries.end_date)} #{tseries.end_date}"
    rdidx = rand(tseries.length);
    v = tseries[rdidx]
    #para "random: #{rdidx}: is  #{v}"
    widget_width = 600
    widget_height = 400
    @surf = stack width: widget_width, height: widget_height do
      background white
      stroke blue
      #top left is 0,0 bottom right is 800,500
      wid = widget_width -20 
      hgt = widget_height - 20
      vscale = hgt / (tsmaxv - tsminv)
      hscale = width / tseries.length
      oldx = 0
      oldy = 0
      (0..tseries.size-1).each do |i|
        v = tseries[i]
        x = (i * hscale).round.to_i
        y = (hgt - ((v - tsminv) * vscale).round).to_i
        if i == 0
          line(x, y, x, y)
        else
          line(oldx, oldy, x, y)
        end
        oldx = x
        oldy = y
      end
    end
    flow do 
      button "quit" do Shoes.quit end
      button "redraw" do
        # how? 
      end
    end
  end
end

