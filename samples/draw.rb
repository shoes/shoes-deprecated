Shoes.app do
  background "rgb(102, 102, 102)"
  stroke 0, 0, 0
  x, y = nil, nil
  motion do |_x, _y|
    if x and y and (x != _x or y != _y)
      append do
        line x, y, _x, _y
      end
    end
    x, y = _x, _y
  end
end
