# svg viewer
Shoes.app width: 400, height: 400, title: "SVG Viewer" do
  fpath = ""
  fl = ''
  @slot = stack do
    tagline "SVG Viewer"
    @display_panel = flow width: 1.0, height: 0.8 do
    end
    @button_panel = flow do
      button "Load" do
        fpath = ask_open_file
        if fpath 
          @display_panel.clear
          @display_panel.append do
            img = svg({:filename => fpath, :width => 200, height: 200})
            puts "#{img.inspect}"
            puts "#{img.methods}"
            #puts "Svg-l,t,w,h: #{img.left},#{img.width},#{img.height}"
            puts "button t: #{self.top}"
            puts "panel t: #{@display_panel.top}"
          end
        end
      end
      button "save" do
      end
      button "default" do
        fpath = "/home/ccoupe/Projects/shoes3/brownshoes.svg"
        fl = File.open(fpath,"r");
        defsvg = fl.read
        fl.close
        @display_panel.clear
        @display_panel.append do
          img = svg({:from_string => defsvg, :width => 200, height: 200})
          puts "#{img.inspect}"
          puts "#{img.methods}"
          #puts "Svg-l,t,w,h: #{img.left},#{img.width},#{img.height}"
          puts "button t: #{self.top}"
          puts "panel t: #{@display_panel.top}"
        end
      end
      button "quit" do
        exit
      end
    end
  end
end
