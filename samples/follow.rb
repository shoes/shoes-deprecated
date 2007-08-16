trails = [[0, 0]] * 60
Shoes.app do
  nostroke
  fill 1.0, 1.0, 1.0, 0.6
  animate(24) do
    trails.shift
    trails << self.mouse[1, 2]

    clear do
      background "rgb(51, 51, 51)"
      trails.each_with_index do |(x, y), i|
        i += 1
        oval x, y, i, i
      end
    end
  end
end
