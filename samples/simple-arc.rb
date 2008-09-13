#
# a translation from a processing example
# http://vormplus.be/weging/an-introduction-to-processing/
#
Shoes.app :width => 420, :height => 420, :resizable => false do
  rotation = -(HALF_PI / 3)
  step = 20 

  background gray(240)
  stroke gray(127)
  cap :curve
  nofill

  10.times do |i|
    strokewidth i
    size = 200 + (step * i)
    shape do
      arc self.width / 2, self.height / 2,
          size, size,
          rotation * i, rotation * i + TWO_PI - HALF_PI
    end
  end
end
