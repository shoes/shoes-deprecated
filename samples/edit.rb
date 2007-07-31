str, t = "", nil
Shoes.app :height => 500, :width => 450 do
  background "rgb(77, 77, 77)"
  stack :margin => 10 do
    text "<span color='white'><span color='red' background='white'>TEXT EDITOR</span> * USE ALT-Q TO QUIT</span>"
  end
  stack :margin => 10 do
    t = text ""
  end
  keypress do |k|
    case k
    when String
      str += k
    when :backspace
      str.slice!(-1)
    when :tab
      str += "  "
    when :alt_q
      quit
    end
    s = Shoes.escape(str).gsub(/^def /, "<span color='#FFDDAA'>def</span> ")
    t.replace "<span font_desc='Monospace 12' color='white'>#{s}</span>"
  end
end
