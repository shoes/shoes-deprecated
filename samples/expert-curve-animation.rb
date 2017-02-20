Shoes.app(:title => "Expert curve_to animation", :resizable => false) do
      xy = [
         [app.slot.width / 6, -app.slot.height],
         [app.slot.width / 6, app.slot.height / 2],
         [app.slot.width / 6, app.slot.height + 50]
      ]
      colors = [
         rgb(49, 156, 0, 0.35),
         rgb(255, 255, 255, 0.35),
         rgb(222, 33, 16, 0.35)
      ]
      
      @waves = stack :top => 0, :left => 0
      animate(8) { |ani|
         a = Math.sin(ani * 0.02) * 8
         @waves.clear do
            nofill
            strokewidth app.slot.width / 3
            6.times { |i|
               colors.each_with_index { |color, n|
                  shape do
                     v = 4.times.collect { rand(0.1) * 100 > 50 ? +1 : -1 }
                     move_to (dx = n * app.slot.width / 3) + xy[n][0] + (v[0] * i * a * 0.8), xy[0][1]
                     stroke color
                     curve_to dx + xy[0][0] + (v[1] * i * a), xy[0][1], dx + xy[1][0] + (v[2] * i * a * 2), xy[1][1], dx + xy[2][0] + (v[3] * i * a), xy[2][1]
                  end
               }
            }
         end
      }
end