Shoes.app width: 700 do
  fpath = "/home/ccoupe/Projects/shoes3/brownshoes.svg"
  cpath = "/home/ccoupe/Projects/shoes3/SVG-cards-2.0.1/paris.svg"
  svgh1 = app.svghandle({:filename => fpath})
  fl = File.open(cpath,"r");
  bigstring = fl.read
  fl.close
  svgh2 = app.svghandle({:content =>bigstring, :group => '#club_1'})
  widget_rect = 200
  flow do
    stack :width => widget_rect, height: widget_rect do
      background gray
      svg 200, 200,svgh1
    end
    stack width: widget_rect, :height => 200 do
      svg 200, 200, {:filename => fpath, :aspect => "no"}
    end
    stack width: 200, height: 200 do
      background gray
      svg 200,200, svgh2
    end
    stack width: 200, height: 200 do
      background cyan
      svg 150, 150, {:filename => fpath}
    end
  end
end


