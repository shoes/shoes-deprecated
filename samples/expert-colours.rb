# https://en.wikipedia.org/wiki/Farnsworth-Munsell_100_hue_test

SQUARES = 22
SQUARE_SIZE = 50
SQUARE_PADDING = 2

TITLE = "Farnsworth-Munsell 100 Hue Color Vision Test [COMPLETED %d%%]"

HUES = [
   [[172, 118, 114], [143, 140, 74]],
   [[143, 140, 74], [84, 151, 137]],
   [[84, 151, 137], [128, 136, 165]],
   [[128, 136, 165], [173, 119, 118]]
]

def generate_rgb(rgb_start, rgb_end, segments = 2)
   rstep, gstep, bstep = 3.times.collect { |n|
      rgb_start[n].to_f.step(
         by: ((rgb_end[n] - rgb_start[n]) / (segments - 1).to_f), 
         to: rgb_end[n]
      ).to_a
   }
   
   segments.times.collect { |n| rgb(rstep[n].floor, gstep[n].floor, bstep[n].floor) }
end

Shoes.app(width: (SQUARE_SIZE + SQUARE_PADDING) * SQUARES + SQUARE_PADDING, height: (SQUARE_SIZE + SQUARE_PADDING) * HUES.size + SQUARE_PADDING, title: TITLE % 0, resizable: false) do
   flow(top: SQUARE_PADDING, left: SQUARE_PADDING / 2, width: (SQUARE_SIZE + SQUARE_PADDING) * SQUARES)  do
      @hues = HUES.collect { |rgb_start, rgb_end| generate_rgb(rgb_start, rgb_end, SQUARES) }
      @hues.collect { |n| [n.first, n[1..-2].shuffle, n.last].flatten }.each_with_index { |list, index|
         SQUARES.times { |n|
               (@r ||= []) << rect(left: (SQUARE_SIZE + SQUARE_PADDING) * n, top: (SQUARE_SIZE + SQUARE_PADDING) * ((n + (index * SQUARES)) / SQUARES), width: SQUARE_SIZE, height: SQUARE_SIZE)
               @r.last.fill = @r.last.stroke = list[n]
         }
      }
   end
   @top_rect = rect(width: SQUARE_SIZE, height: SQUARE_SIZE).hide
   
   motion do |left, top|
      @top_rect.move(left - (@controller.width / 2), top - (@controller.height / 2)) unless @controller.nil?
   end

   @anim = animate(10) do |frame|
      if 0 == frame % 5
         set_window_title(TITLE % (@completed * 100 / @r.size)) unless @completed.nil?
         if @highlight
            @hues.flatten.each_with_index { |item, index|
               @r[index].stroke = item.eql?(@r[index].fill) ? @r[index].fill : rgb(255, 0, 0)
            }
         else
            @r.each { |n| n.stroke = n.fill }
         end
      end
      if @success
         5.times {
            a, b = rand(@r.size), rand(@r.size)
            @r[a].fill, @r[b].fill = @r[b].fill, @r[a].fill
         }
      end
   end
   
   keypress do |k|
      @highlight ^= true if k.eql?("h")
   end
   
   start do
      @highlight = false
      @r.each_with_index { |n, skip|
         next if 0 == skip % SQUARES or (SQUARES - 1) == (skip % SQUARES)
         n.click do |btn, left, top|
            if @success
               set_window_title(TITLE % (@completed = 0))
               @hues.collect { |n| [n.first, n[1..-2].shuffle, n.last].flatten }.flatten.each_with_index { |item, index|
                  @r[index].fill = @r[index].stroke = item
               }
               @success = false
               @top_rect.hide unless @top_rect.nil?
               @controller = nil
               next
            end
            
            unless @controller.nil?
               if n.top.eql?(@controller.top)
                  n.fill, @controller.fill = @controller.fill, n.fill
                  n.stroke, @controller.stroke = @controller.stroke, n.stroke
                  
                  @top_rect.hide
                  @controller = nil
               end
            else
               @controller = n
               @top_rect.left, @top_rect.top = n.left, n.top
               @top_rect.fill, @top_rect.stroke = n.fill, n.stroke
               @top_rect.show
            end
            
            @success = true
            @completed = 0
            @hues.flatten.each_with_index { |item, index|
               item.eql?(@r[index].fill) ? @completed += 1 : @success = false
            }
         end
      }
   end
end