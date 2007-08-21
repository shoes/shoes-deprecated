trails = [[0, 0]] * 60
Shoes.app do
  nostroke
  fill rgb(0x30, 0xFF, 0xFF, 0.6)

  # animation at 24 frames per second
  animate(24) do
    trails.shift
    trails << self.mouse[1, 2]

    clear do
      # change the background based on where the pointer is
      background rgb(
        20 + (70 * (trails.last[0].to_f / self.width)).to_i, 
        20 + (70 * (trails.last[1].to_f / self.height)).to_i,
        51)

      # draw circles progressively bigger
      trails.each_with_index do |(x, y), i|
        i += 1
        oval :left => x, :top => y, :radius => i, :center => true
      end
    end
  end

end
