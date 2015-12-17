# svg viewer
Shoes.app width: 500, height: 600, title: "SVG Viewer" do
  fpath = ""
  fl = ''
  widget_size = 480
  defaspect = true
  @slot = stack do
    tagline "SVG Viewer"
    @display_panel = flow width: widget_size, height: widget_size do
    end
    @button_panel = flow do
      button "Load" do
        fpath = ask_open_file
        if fpath 
          @display_panel.clear
          @display_panel.append do
            @current_svg = svg  widget_size, widget_size, {:filename => fpath, aspect: defaspect}
          end
        end
      end
      button "save" do
      end
       button "quit" do
        exit
      end
      @subid = edit_line :width=> 120, text: "all"
      button "use group" do
        id = nil
        if @subid.text != "all"
          if (@subid.text[0] != '#') && (confirm "Did you mean \##{@subid.text}")
            id = "#"+@subid.text
            @subid.text = id
          else
            id = @subid.text
          end
        end
        # we should have a svg.group?(str) method
        @display_panel.clear
        @display_panel.append do
          @current_svg = svg  widget_size, widget_size, {:filename => fpath, aspect: @aspect.checked?, group: id}
        end
      end
      stack do
        inscription "Aspect"
        flow do
          @aspect = check checked: true do
            puts "aspect #{@aspect.checked?}"
            @display_panel.clear
            @display_panel.append do
              @current_svg = svg  widget_size, widget_size, {:filename => fpath, aspect: @aspect.checked?}
            end
          end 
          para "use image aspect"
        end
        flow do
          button "Custom aspect:" 
          @specified = edit_line :width => 50, text: "1.00"
        end
      end
    end
  end
end
