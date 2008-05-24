class MenuPanel < Widget
  @@boxes = []
  def initialize(color, args)
    @@boxes << self
    background color
    para link("Box #{@@boxes.length}", :stroke => white, :fill => nil).
      hover { expand }.
      click { visit "/" },
        :margin => 18, :align => "center", :size => 20
  end
  def expand
    if self.width < 170
      a = animate 30 do
        @@boxes.each do |b|
          b.width -= 5 if b != self and b.width > 140
        end
        self.width += 5
        self.show
        a.stop if self.width >= 170
      end
    end
  end
end

Shoes.app :width => 400, :height => 130 do
  style(LinkHover, :fill => nil)
  menupanel green,  :width => 170, :height => 120, :margin => 4
  menupanel blue,   :width => 140, :height => 120, :margin => 4
  menupanel red,    :width => 140, :height => 120, :margin => 4
  menupanel purple, :width => 140, :height => 120, :margin => 4
end
