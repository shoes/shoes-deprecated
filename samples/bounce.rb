size = 60
xspeed, yspeed = 8.4, 6.6
xdir, ydir = 1, 1

Shoes.app do
  nostroke
  x, y = self.width / 2, self.height / 2
  animate(30) do
    clear do
      background greenyellow
      x += xspeed * xdir
      y += yspeed * ydir
    
      xdir *= -1 if x > self.width - size or x < 0
      ydir *= -1 if y > self.height - size or y < 0

      oval :left => x + size / 2, :top => y + size / 2, :radius => size, :center => true
    end
  end
end
