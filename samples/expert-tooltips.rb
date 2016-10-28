class Tooltip < Shoes::Widget

    def initialize ()
        motion { |x, y| @move_left = x; @move_top = y }
        @menu1 = stack;
    end

    def show ( options={} )
        @menu1.clear { @menu = stack height: 0 do end.hide }
        offset, high, wide, text, stroke, size, font, back1, back2 = 10, options[:height], options[:width], options[:text], options[:stroke], options[:size], options[:font],  options[:border] || rgb(120,42,5,0.5), options[:background] || rgb(180,150,110,0.7) 
        ( wide.nil? || high.nil? || text.nil? ) ? ( high = 250; wide = 400; size = 10; text = "ERROR: Missing mandatory options \"width:, \"height:\" or \"text:\"\nAvailable options:\ntext: the text presented in the tooltip window\nwidth: the width of the tooltip\nheight: the height of the tooltip font: the font of the text\nstroke: the stroke of the text\nsize: the size of the text\nborder: the colour of the border of the tooltip\nbackground: the colour of the tooltip box\nExample: slot.hover { @tooltip.show text: \"tooltip comming through\", width: 200, height: 100, border: rgb(0,20,5,0.5), background: rgb(0,244,99,0.5) , font: Vivaldi , size: 10, stroke: white \} "  ) : nil
        @menu.style width: offset + wide, height: offset + high
        menu_left, menu_top = @move_left, @move_top;
        @move_left + @menu.width >= app.width + offset ? menu_left = @move_left - @menu.width : nil
        @move_top + @menu.height >= app.height + offset ? menu_top = @move_top - 2*@menu.height/3 : nil
        @menu.move(menu_left, menu_top)
        @menu.clear do
            background back1, curve: 15
            background back2, curve: 15, width: @menu.width-4, height: @menu.height - 4, left: 2, top: 2
            flow margin_left: offset, width: wide, height: high do
                para ("#{text}"), size: size, stroke: stroke, font: font, align: "center", justify: true
            end
        end
        start { @menu.show }
    end

    def hide
        @menu.hide
    end
end 

Shoes.app do
background palegreen
para "Demonstrates how to write tool tips. Credit and thanks to @dredknight\n" \
 "Move the mouse to the icons below" 
@hovers = tooltip()

    def set  (img, options={} )
            img.hover { @hovers.show options } 
            img.leave { @hovers.hide; }
    end
    set ( image "#{DIR}/static/app-icon.png", width: 50, height: 50, left: 30, top: 100 ), text: "No options"
    set ( image "#{DIR}/static/app-icon.png", width: 50, height: 50, left: 120, top: 100 ), text: "Mandatory settings only", width: 200, height: 200
    set ( image "#{DIR}/static/app-icon.png", width: 50, height: 50, left: 340, top: 100 ), text: "ALL SETTINGS", width: 200, height: 200, stroke: orange, size: 14, font: "Serif", border: blue, background: yellow
end
