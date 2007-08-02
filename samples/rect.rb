rectangles = proc do
  20.times do
    nostroke
    fill rand(0.6) + 0.4, rand(0.1) + 0.9, rand(0.2) + 0.8, rand(0.4) + 0.1
    r = rand(300) + 60
    rect rand(100), rand(200), r, r
  end
  button "OK", :x => 300, :y => 400 do
    clear
    append &rectangles
  end
end

Shoes.app &rectangles
