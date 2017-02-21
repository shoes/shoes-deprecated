# https://github.com/casparjones/First-Ruby-Steps/blob/master/shoes-samples/simple-curve.rb
# http://www.cairographics.org/samples/curve_to/

# curve_to with visible control points.
# The ovals represent the control points.
Shoes.app(title: "Expert curve_to control points", width: 500, height: 400) do
   xy = [
      [self.width / 8, self.height / 2], 
      [self.width / 8 * 3, self.height / 4 * 3], 
      [self.width / 8 * 4, self.height / 4], 
      [self.width / 8 * 7, self.height / 2]
   ]

   para "click and drag control points..."
   fill green(0.2)
   move_to *xy[0]
   
   @controller = nil
   xy.each_with_index { |val, index|
      (@controllers ||= []) << oval(:left => val[0], :top => val[1], :radius => 10, :center => true)
   }
   @controllers.each_with_index { |n, index|
      n.click do |btn, left, top|
         n.style :fill => red
         @controller = n
      end
      n.release do |btn, left, top|
         n.style :fill => green(0.2)
         @controller = nil
      end
   }
   
   motion do |left, top|
      unless @controller.nil?
         xy[@controllers.index(@controller)] = left, top
         @controller.left, @controller.top = left, top
      end
   end
   
   @stack = stack :top => 0, :left => 0
   
   animate(10) do
      @stack.clear do
         fill red(0.2)   
         shape do
            move_to *xy[0]
            curve_to *xy[1], *xy[2], *xy[3]
         end
      end
   end
end