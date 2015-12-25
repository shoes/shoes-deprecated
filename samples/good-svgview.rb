# svg viewer
require 'nokogiri'
Shoes.app width: 500, height: 600, title: "SVG Viewer" do
  fpath = ""
  widget_size = 400
  defaspect = true
  @xml = ''
  @ids = []
  def load (path)
    File.open(path) {|f| @xml = f.read}
    doc = Nokogiri::XML(@xml)
    @ids = []
    #doc.css("g").each_with_object("") { |tag, str| @ids << tag["id"].inspect }
    ids = doc.css("g[id]")
    def_ids = doc.css("defs g[id]")
    groups = ids - def_ids
    groups.each_with_object("") { |elem, str| @ids << elem.attributes["id"].value.strip }
    #puts @ids
  end
  @slot = stack do
    tagline "SVG Viewer"
    @display_panel = flow width: widget_size, height: widget_size do
    end
    @button_panel = flow do
      button "Load" do
        fpath = ask_open_file
        if fpath 
          load(fpath)
          @lb.items = @ids
          @display_panel.clear
          @display_panel.append do
            @current_svg = svg  @xml, {width: widget_size, height: widget_size, aspect: defaspect}
          end
        end
      end
      button "Paris Cards" do
        fpath = ''
        if Shoes::RELEASE_TYPE =~ /TIGHT/
          fpath = "#{DIR}/samples/paris.svg"
        else
          fpath = "#{DIR}/../samples/paris.svg"
        end
        if ! File.exist? fpath 
          alert "Can't find #{fpath} - crash ahead"
        end
        load(fpath)
        @lb.items = @ids
        @display_panel.clear
        @display_panel.append do
          @current_svg = svg @xml, { width: widget_size, height: widget_size}
        end
      end
       button "quit" do
        exit
      end
      @lb = list_box items: @ids do
        @subid.text = '#'+@lb.text
      end
      @subid = edit_line :width=> 120, text: "all"
      button "use group" do
        id = @subid.text
        id = '' if id == 'all'
        if id == '' || id == nil || (@current_svg.group? id)
          @display_panel.clear
          @display_panel.append do
            @current_svg = svg fpath, {width: widget_size, height: widget_size, aspect: @aspect.checked?, group: id}
          end
        end
      end
      stack do
        inscription "Aspect"
        flow do
          @aspect = check checked: true do
            puts "aspect #{@aspect.checked?}"
            @display_panel.clear
            @display_panel.append do
              @current_svg = svg  fpath, {width: widget_size, height: widget_size, aspect: @aspect.checked?}
            end
          end 
          para "use image aspect"
        end
        flow do
          button "Custom aspect:"  do
            asp = eval(@specified.text).to_f
            @aspect.checked = false
            @display_panel.clear
            @display_panel.append do
              @current_svg = svg  fpath, widget_size, widget_size, { aspect: asp}
            end
          end
         @specified = edit_line :width => 50, text: "1.00" 
        end
      end
    end
  end
end
