Shoes.app width: 600 do
  fpath = "/home/ccoupe/Projects/shoes3/brownshoes.svg"
  cpath = "/home/ccoupe/Projects/shoes3/SVG-cards-2.0.1/paris.svg"
  svgh1 = app.svghandle({:filename => fpath})
  fl = File.open(cpath,"r");
  bigstring = fl.read
  fl.close
  svgh2 = app.svghandle({:content =>bigstring, :subid => '#diamond_queen'})
  widget_rect = 200
  flow do
    stack :width => widget_rect do
      w1 = svgh1.width 
      h1 = svgh1.height
      a1 = (w1.to_f) / h1
      para "w: #{w1} h: #{h1}"
      para "a: #{a1}"
      nw1 = widget_rect
      nh1 = (widget_rect * (1 / a1)).to_i
      para "nw #{nw1} nh: #{nh1}"
      para "na: #{nw1.to_f / nh1}"
      svg nw1,nh1,svgh1
    end
    stack width: widget_rect do
      w2 = svgh2.width 
      h2 = svgh2.height
      a2 = w2.to_f / h2
      nw2 = (widget_rect * a2).to_i
      nh2 = widget_rect
      para "w: #{w2} h: #{h2}"
      para "a: #{a2}"
      para "nw #{nw2} nh: #{nh2}"
      para "na: #{nw2.to_f / nh2}"
      svg 200,200,svgh2
    end
    stack width: widget_rect do
      svg 100, 100, {:filename => fpath}
    end
  end
end


