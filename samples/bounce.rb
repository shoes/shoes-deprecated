size = 60
xspeed, yspeed = 2.8, 2.2
xdir, ydir = 1, 1

Shoes.app do
  nostroke
  x, y = self.width / 2, self.height / 2
  animate(30) do
    clear do
      background "rgb(102, 102, 102)"
      x += xspeed * xdir
      y += yspeed * ydir
    
      xdir *= -1 if x > self.width - size or x < 0
      ydir *= -1 if y > self.height - size or y < 0

      oval x + size / 2, y + size / 2, size, size
    end
  end
end
