# svg viewer
require 'nokogiri'

Shoes.app width: 500, height: 610, title: "SVG Viewer" do
  @widget_sizeW = 450
  @widget_sizeH = 400

  def load (path)
    File.open(path) {|f| @xml = f.read}
    doc = Nokogiri::XML(@xml)
    groups = doc.css("g[id]") - doc.css("defs g[id]")
    groups.each_with_object([]) { |elem, arr| arr << elem.attributes["id"].value.strip }
  end

  def get_details
    "original svg offsets : x = #{@current_svg.offset_x}, y = #{@current_svg.offset_y} \n" +
    "original svg size    : width = #{@current_svg.preferred_width}, height = #{@current_svg.preferred_height}"
  end

  def show_svg(fpath)
    @lbw.items = load(fpath)
    @display_panel.clear do
      @current_svg = svg @xml, width: @widget_sizeW, height: @widget_sizeH, aspect: true
      if @current_svg.is_a? Array
        #puts "Converting array"
        @current_svg = @current_svg[0]
      end
    end
    @details.text = get_details
    @group.checked = false
    @subid.text = ""
    @aspect.checked = true
    @specified.text = "1.0"
  end

  stack do
    flow do
        tagline "SVG Viewer", margin: [0,0,30,0]
        @details = inscription
    end

    @display_panel = flow width: @widget_sizeW+20, height: @widget_sizeH+20, margin: 10 do; end

    flow margin: [5,20,0,10] do
      button "Load" do
        if fpath = ask_open_file
          show_svg(fpath)
        end
      end

      button "Paris Cards" do
        fpath = Shoes::RELEASE_TYPE =~ /TIGHT/ ? "#{DIR}/samples/paris.svg" : "#{DIR}/../samples/paris.svg"

        if File.exist? fpath
          show_svg(fpath)
        else
          alert "Can't find #{fpath} !!"
        end
      end

      button "Render", width: 150, margin: [25,0,0,0] do
        return if @xml.nil? || @xml.empty?

        id = @subid.text
        id = (@group.checked? && @current_svg.group?(id)) ? id : nil

        asp = @specified.text.to_f == 1.0 ? true : @specified.text.to_f
        asp = @aspect.checked? ? asp : false
        @display_panel.clear do
          @current_svg = svg @xml, width: @widget_sizeW, height: @widget_sizeH, group: id, aspect: asp
          if @current_svg.is_a? Array
            @current_svg = @current_svg[0]
            #puts "Converted in Render"
          end
        end
        @details.text = get_details
      end
      
      button "Save" do
        savep = ask_save_file
        if savep
          ext = File.extname(savep)
          case ext
          when  '.png'
            @current_svg.export filename: savep
          when '.svg', '.pdf', '.ps'
            @current_svg.save filename: savep, format: ext
          else
            alert "not png, svg, ps or pdf"
          end
        end
      end

      button "Quit", margin: [10,0,0,0] do; Shoes.quit; end
    end

    flow do
      @group = check checked: false
      para "use image group", margin_right: 20
      @subid = edit_line :width => 120, text: ""

      @lbw = list_box do |lb|
        @subid.text = '#' + (lb.text.nil? ? "": lb.text)
      end
    end

    flow do
      @aspect = check checked: true
      para "use image aspect", margin_right: 16
      @specified = edit_line :width => 100, text: "1.00"
    end
  end
end
