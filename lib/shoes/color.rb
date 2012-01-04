class Shoes::Types::Color
  attr_accessor :r, :g, :b, :a

  def initialize(r_or_color, g, b, a=255)
    @r = r
    @g = g
    @b = b
    @a = a    
  end

  def self.rgb(r, g, b, a=255)
    new(r, g, b, a)
  end

  # parses out a color
  def self.parse(source)
  end

  include Comparable

  def <=>(other)
    alert("fffuuu")
    return nil unless other.kind_if?(Color)

    alert("zomg!")
    if r == other.r and g == other.g and b == other.b
      return 0
    else
      v1 = r + g + b
      v2 = other.r + other.g + other.b
      return 0 if v1 == v2
      return 1 if v1 > v2
      return -1
    end
  end

  def red
    r
  end

  def green
    g
  end

  def blue
    b
  end

  def alpha
    a
  end

  def black?
    r + g + b == 0
  end

  def dark?
    r + g + b < 255
  end

  def light?
    r + g + b > 510
  end

  def opaque?
    a == 255
  end

  def transparent?
    a == 0
  end

  def white?
    r + g + b == 765
  end

  def invert
    new(255 - r, 255 - g, 255 - b, 255)
  end

  def to_s
    if a == 255
      "rgb(%s, %s, %s)" % [r, g, b]
    else
      "rgb(%s, %s, %s, %0.1f)" % [r, g, b, a / 255.0]
    end
  end

  def to_pattern
    pattern(self)
  end

  def inspect
    to_s
  end
end

module Kernel
  def rgb(r, g, b, a=255)
    Shoes::Color.rgb(r, g, b, a)
  end
  def gray(num)
    Shoes::Color.new(num, num, num, 255)
  end
end

aliceblue = Shoes::Color.new(240, 248, 255)
antiquewhite = Shoes::Color.new(250, 235, 215)
aqua = Shoes::Color.new(0, 255, 255)
aquamarine = Shoes::Color.new(127, 255, 212)
azure = Shoes::Color.new(240, 255, 255)
beige = Shoes::Color.new(245, 245, 220)
bisque = Shoes::Color.new(255, 228, 196)
black = Shoes::Color.new(0, 0, 0)
blanchedalmond = Shoes::Color.new(255, 235, 205)
blue = Shoes::Color.new(0, 0, 255)
blueviolet = Shoes::Color.new(138, 43, 226)
brown = Shoes::Color.new(165, 42, 42)
burlywood = Shoes::Color.new(222, 184, 135)
cadetblue = Shoes::Color.new(95, 158, 160)
chartreuse = Shoes::Color.new(127, 255, 0)
chocolate = Shoes::Color.new(210, 105, 30)
coral = Shoes::Color.new(255, 127, 80)
cornflowerblue = Shoes::Color.new(100, 149, 237)
cornsilk = Shoes::Color.new(255, 248, 220)
crimson = Shoes::Color.new(220, 20, 60)
cyan = Shoes::Color.new(0, 255, 255)
darkblue = Shoes::Color.new(0, 0, 139)
darkcyan = Shoes::Color.new(0, 139, 139)
darkgoldenrod = Shoes::Color.new(184, 134, 11)
darkgray = Shoes::Color.new(169, 169, 169)
darkgreen = Shoes::Color.new(0, 100, 0)
darkkhaki = Shoes::Color.new(189, 183, 107)
darkmagenta = Shoes::Color.new(139, 0, 139)
darkolivegreen = Shoes::Color.new(85, 107, 47)
darkorange = Shoes::Color.new(255, 140, 0)
darkorchid = Shoes::Color.new(153, 50, 204)
darkred = Shoes::Color.new(139, 0, 0)
darksalmon = Shoes::Color.new(233, 150, 122)
darkseagreen = Shoes::Color.new(143, 188, 143)
darkslateblue = Shoes::Color.new(72, 61, 139)
darkslategray = Shoes::Color.new(47, 79, 79)
darkturquoise = Shoes::Color.new(0, 206, 209)
darkviolet = Shoes::Color.new(148, 0, 211)
deeppink = Shoes::Color.new(255, 20, 147)
deepskyblue = Shoes::Color.new(0, 191, 255)
dimgray = Shoes::Color.new(105, 105, 105)
dodgerblue = Shoes::Color.new(30, 144, 255)
firebrick = Shoes::Color.new(178, 34, 34)
floralwhite = Shoes::Color.new(255, 250, 240)
forestgreen = Shoes::Color.new(34, 139, 34)
fuchsia = Shoes::Color.new(255, 0, 255)
gainsboro = Shoes::Color.new(220, 220, 220)
ghostwhite = Shoes::Color.new(248, 248, 255)
gold = Shoes::Color.new(255, 215, 0)
goldenrod = Shoes::Color.new(218, 165, 32)
gray = Shoes::Color.new(128, 128, 128)
green = Shoes::Color.new(0, 128, 0)
greenyellow = Shoes::Color.new(173, 255, 47)
honeydew = Shoes::Color.new(240, 255, 240)
hotpink = Shoes::Color.new(255, 105, 180)
indianred = Shoes::Color.new(205, 92, 92)
indigo = Shoes::Color.new(75, 0, 130)
ivory = Shoes::Color.new(255, 255, 240)
khaki = Shoes::Color.new(240, 230, 140)
lavender = Shoes::Color.new(230, 230, 250)
lavenderblush = Shoes::Color.new(255, 240, 245)
lawngreen = Shoes::Color.new(124, 252, 0)
lemonchiffon = Shoes::Color.new(255, 250, 205)
lightblue = Shoes::Color.new(173, 216, 230)
lightcoral = Shoes::Color.new(240, 128, 128)
lightcyan = Shoes::Color.new(224, 255, 255)
lightgoldenrodyellow = Shoes::Color.new(250, 250, 210)
lightgreen = Shoes::Color.new(144, 238, 144)
lightgrey = Shoes::Color.new(211, 211, 211)
lightpink = Shoes::Color.new(255, 182, 193)
lightsalmon = Shoes::Color.new(255, 160, 122)
lightseagreen = Shoes::Color.new(32, 178, 170)
lightskyblue = Shoes::Color.new(135, 206, 250)
lightslategray = Shoes::Color.new(119, 136, 153)
lightsteelblue = Shoes::Color.new(176, 196, 222)
lightyellow = Shoes::Color.new(255, 255, 224)
lime = Shoes::Color.new(0, 255, 0)
limegreen = Shoes::Color.new(50, 205, 50)
linen = Shoes::Color.new(250, 240, 230)
magenta = Shoes::Color.new(255, 0, 255)
maroon = Shoes::Color.new(128, 0, 0)
mediumaquamarine = Shoes::Color.new(102, 205, 170)
mediumblue = Shoes::Color.new(0, 0, 205)
mediumorchid = Shoes::Color.new(186, 85, 211)
mediumpurple = Shoes::Color.new(147, 112, 219)
mediumseagreen = Shoes::Color.new(60, 179, 113)
mediumslateblue = Shoes::Color.new(123, 104, 238)
mediumspringgreen = Shoes::Color.new(0, 250, 154)
mediumturquoise = Shoes::Color.new(72, 209, 204)
mediumvioletred = Shoes::Color.new(199, 21, 133)
midnightblue = Shoes::Color.new(25, 25, 112)
mintcream = Shoes::Color.new(245, 255, 250)
mistyrose = Shoes::Color.new(255, 228, 225)
moccasin = Shoes::Color.new(255, 228, 181)
navajowhite = Shoes::Color.new(255, 222, 173)
navy = Shoes::Color.new(0, 0, 128)
oldlace = Shoes::Color.new(253, 245, 230)
olive = Shoes::Color.new(128, 128, 0)
olivedrab = Shoes::Color.new(107, 142, 35)
orange = Shoes::Color.new(255, 165, 0)
orangered = Shoes::Color.new(255, 69, 0)
orchid = Shoes::Color.new(218, 112, 214)
palegoldenrod = Shoes::Color.new(238, 232, 170)
palegreen = Shoes::Color.new(152, 251, 152)
paleturquoise = Shoes::Color.new(175, 238, 238)
palevioletred = Shoes::Color.new(219, 112, 147)
papayawhip = Shoes::Color.new(255, 239, 213)
peachpuff = Shoes::Color.new(255, 218, 185)
peru = Shoes::Color.new(205, 133, 63)
pink = Shoes::Color.new(255, 192, 203)
plum = Shoes::Color.new(221, 160, 221)
powderblue = Shoes::Color.new(176, 224, 230)
purple = Shoes::Color.new(128, 0, 128)
red = Shoes::Color.new(255, 0, 0)
rosybrown = Shoes::Color.new(188, 143, 143)
royalblue = Shoes::Color.new(65, 105, 225)
saddlebrown = Shoes::Color.new(139, 69, 19)
salmon = Shoes::Color.new(250, 128, 114)
sandybrown = Shoes::Color.new(244, 164, 96)
seagreen = Shoes::Color.new(46, 139, 87)
seashell = Shoes::Color.new(255, 245, 238)
sienna = Shoes::Color.new(160, 82, 45)
silver = Shoes::Color.new(192, 192, 192)
skyblue = Shoes::Color.new(135, 206, 235)
slateblue = Shoes::Color.new(106, 90, 205)
slategray = Shoes::Color.new(112, 128, 144)
snow = Shoes::Color.new(255, 250, 250)
springgreen = Shoes::Color.new(0, 255, 127)
steelblue = Shoes::Color.new(70, 130, 180)
tan = Shoes::Color.new(210, 180, 140)
teal = Shoes::Color.new(0, 128, 128)
thistle = Shoes::Color.new(216, 191, 216)
tomato = Shoes::Color.new(255, 99, 71)
turquoise = Shoes::Color.new(64, 224, 208)
violet = Shoes::Color.new(238, 130, 238)
wheat = Shoes::Color.new(245, 222, 179)
white = Shoes::Color.new(255, 255, 255)
whitesmoke = Shoes::Color.new(245, 245, 245)
yellow = Shoes::Color.new(255, 255, 0)
yellowgreen = Shoes::Color.new(154, 205, 50)

