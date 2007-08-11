Shoes.app do
  l = edit_line
  animate do |i|
    l.text = i.inspect
  end
  motion do |x, y|
    p [x, y]
  end
end
