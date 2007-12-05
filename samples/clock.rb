#
# Shoes Clock by Thomas Bell
# posted to the Shoes mailing list on 04 Dec 2007
#
Shoes.app :height => 250, :width => 250 do
  @radius, @centerx, @centery = 90, 120, 130
  stack :margin => 10 do
    animate(8) do
      @time = Time.now
      clear do
        draw_background
        para @time.strftime("%a %b %d, %Y %I:%M:%S"), :align => "center"
        clock_hand @time.sec + (@time.usec * 0.000001),2,30,red
        clock_hand @time.min + (@time.sec / 60.0),5
        clock_hand @time.hour + (@time.min / 60.0),8,6
      end
    end
  end
  def draw_background
    background rgb(240, 250, 210)

    fill white
    oval @centerx - 100, @centery - 100, 200, 200

    fill black
    nostroke
    oval @centerx - 5, @centery - 5, 10, 10

    stroke black
    line(@centerx, @centery - 100, @centerx, @centery - 95)
    line(@centerx - 100, @centery, @centerx - 95, @centery)
    line(@centerx + 95, @centery, @centerx + 100, @centery)
    line(@centerx, @centery + 95, @centerx, @centery + 100)
  end
  def clock_hand(time, sw, unit=30, color=black)
    radius_local = unit == 30 ? @radius : @radius - 15
    _x = radius_local * Math.sin( time * Math::PI / unit )
    _y = radius_local * Math.cos( time * Math::PI / unit )
    stroke color
    strokewidth sw
    line(@centerx, @centery, @centerx + _x, @centery - _y)
  end
end
