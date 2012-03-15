WIDTH = 400
HEIGHT = 400

Shoes.app :width => WIDTH, :height => HEIGHT, :resizable => false do
  background hotpink
  stack do
    #@argon = banner "ARGON FLYER", :stroke => lawngreen, :align => 'center'
    @argon = image "argon_flyer.png"
  end

  animate 10 do
    left = new_position(WIDTH, @argon.width, @argon.left)
    top = new_position(HEIGHT, @argon.height, @argon.top)
    @argon.move left, top
  end

  def keep_in_frame(frame_max, object_size, starting_point, position)
    if (position < 0) 
      position += 2
    elsif position + object_size > frame_max
      position -= 2
    end
    position
  end

  def new_position(frame_max, object_size, starting_point)
    position = random_position(starting_point)
    keep_in_frame(frame_max, object_size, starting_point, position)
  end

  def random_position(starting_point)
    starting_point + (random_amount * random_direction)
  end

  def random_amount
    rand(20)
  end

  def random_direction
    rand(2) == 1 ? 1 : -1
  end
end
