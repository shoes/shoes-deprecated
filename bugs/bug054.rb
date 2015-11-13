# simple test of svghandle class - full path name is important
Shoes.app do
  @slot = stack do
    fpath = "/home/ccoupe/Projects/shoes3/icon/brownshoes.svg"
    #svgh = svghandle({:filename => fpath})
    #svgh.close
    fl = File.open(fpath,"r");
    bigstring = fl.read
    fl.close
    para "SVG is #{bigstring.length} long"
    svg2 = svghandle({:from_string => bigstring})
    button "draw" do
      svg2.draw
      #svg2.close
    end
  end
end
