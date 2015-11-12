# simple test of svghandle class - full path name is important
Shoes.app do
  fpath = "/home/ccoupe/Projects/shoes3/icon/brownshoes.svg"
  svgh = svghandle({:filename => fpath})
  svgh.close
  fl = File.open(fpath,"r");
  bigstring = fl.read
  fl.close
  para "SVG is #{bigstring.length} long"
  sgvg2 = svghandle({:from_string => bigstring})
  svg2.close
end
