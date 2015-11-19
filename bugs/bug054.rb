# simple test of svghandle class - full path name is important
Shoes.app width: 400, height: 400 do
  @slot = stack do
    fpath = "/home/ccoupe/Projects/shoes3/brownshoes.svg"
    #svgh = svghandle({:filename => fpath})
    #svgh.close
    fl = File.open(fpath,"r");
    bigstring = fl.read
    fl.close
    para "SVG is #{bigstring.length} bytes"
    @svg2 = svg({:from_string => bigstring})
    button "right-down" do
      puts "ltwh: #{@svg2.left},#{@svg2.top},#{@svg2.width},#{@svg2.height}"
      @svg2.move(@svg2.left+10,@svg2.top+10)
    end
  end
end
