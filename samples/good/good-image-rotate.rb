Shoes.app(width: 256, height: 256, resizable: false) {
  stack {
    @img = image("#{DIR}/static/shoes-icon-walkabout.png")
    @counter = 1
    @interpolator = (tmp = (0..50).collect { |n| -n }) + tmp.reverse
    animate(@fps = 60) { |frame|
      @img.rotate(@interpolator.first)
      if ((frame / @fps) == @counter)
        @counter += 1
        @interpolator.push @interpolator.shift
      end
    }
  }
}