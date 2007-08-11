Shoes.app do
  l = text "0"
  animate(24) do |i|
    l.replace i.inspect
  end
  # motion do |x, y|
  #   Shoes.p [x, y]
  # end
end
