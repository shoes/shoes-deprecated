# https://github.com/casparjones/First-Ruby-Steps/blob/master/shoes-samples/simple-curve.rb
# http://www.cairographics.org/samples/curve_to/

# curve_to with visible control points.
# The ovals represent the control points.
Shoes.app(title: "curve_to control points", width: 256, height: 256) do
   x, y = 25.6, 128.0
   x1 = 102.4; y1 = 230.4
   x2 = 153.6; y2 = 25.6
   x3 = 230.4; y3 = 128.0

   fill green(0.2)
   move_to x, y
   oval(:left => x, :top => y, :radius => 10, :center => true)
   oval(:left => x1, :top => y1, :radius => 10, :center => true)
   oval(:left => x2, :top => y2, :radius => 10, :center => true)
   oval(:left => x3, :top => y3, :radius => 10, :center => true)
   
   fill red(0.2)   
   shape do
      move_to x, y
      curve_to x1, y1, x2, y2, x3, y3
   end
end