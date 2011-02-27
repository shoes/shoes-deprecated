require 'minitest/autorun'

# We need to do this to test just the Ruby code.
class Shoes
  class Types;end
  class Background;end
  class Border;end
  class Canvas;end
  class Check;end
  class Radio;end
  class EditLine;end
  class EditBox;end
  class Effect;end
  class Image;end
  class ListBox;end
  class Progress;end
  class Shape;end
  class TextBlock;end
  class Text;end
end

# from shoes/world.c
DIR = File.expand_path(File.dirname(__FILE__));

require_relative '../lib/shoes'
