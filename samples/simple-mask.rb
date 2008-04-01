Shoes.app do
  20.times do |i|
    strokewidth 4
    stroke rgb((0.0..0.5).rand, (0.0..1.0).rand, (0.0..0.3).rand)
    line 0, i * 4, 400, i * 4
  end

  mask do
    title "Shoes"
  end
end
