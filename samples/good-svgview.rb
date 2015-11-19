# svg viewer
Shoes.app width: 600, height: 400, title: "SVG Viewer" do
  fpath = "/home/ccoupe/Projects/shoes3/brownshoes.svg"
  fl = File.open(fpath,"r");
  defsvg = fl.read
  fl.close
  @slot = stack do
    tagline "SVG Viewer"
    @display_panel = flow width: 1.0, height: 0.8 do
    end
    @button_panel = flow do
      button "Load" do
      end
      button "save" do
      end
      button "default" do
        @display_panel.clear
        @display_panel.append do
          img = svg({:from_string => defsvg})
          puts "Svg-w,h: #{img.width},#{img.height}"
        end
      end
      button "quit" do
        exit
      end
    end
  end
end
