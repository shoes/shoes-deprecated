Shoes.app(title: "Drag the dot to make a path. Redrag to extend.") do
   @path = [[self.width / 2, self.height / 2, RADIUS = 20]]
   
   @controller = nil
   click do |btn, left, top|
      px, py, pr = @path.last
      if left.between?(px - pr, px + pr) and top.between?(py - pr, py + pr)
         @controller = true
      end
   end
   release do |btn, left, top|
      @controller = nil
   end

   motion do |left, top|
      @path << [left, top, RADIUS] unless @controller.nil?
   end
   
   button("reset") do
      @index = 0
      @path = [@path.first]
      @controller = nil
   end
   
   @stack = stack :top => 0, :left => 0

   @index = 0
   animate(24) do
      @stack.clear do
         fill red(0.05)
         stroke red(0.05)
         ovals = @path.collect { |x, y, r| oval x, y, r, :center => true }
         ovals.first.style :fill => fuchsia, :stroke => fuchsia
         ovals.last.style :fill => blue, :stroke => blue
         
         if @controller.nil? and @path.size > 1
            fill green
            stroke green
            rotate -5
            x, y, r = @path[@index]
            rect x - 40, y - 40, r * 4, r * 4, r / 2
            @index = (@index < @path.size - 1) ? @index + 1 : 0
         end
      end
   end
end