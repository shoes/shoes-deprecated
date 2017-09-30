MAXWIDTH = 1600
MAXHEIGHT = 900

Shoes.app(width: 300, height: 100) do
    @width, @height = app.width, app.height
    stack margin: 10 do
        flow do
            slider fraction: (@width / MAXWIDTH.to_f) do |s|
                @width = (s.fraction * MAXWIDTH)
            end
            para "width", margin_left: 10
        end
        flow do
            slider fraction: (@height / MAXHEIGHT.to_f) do |s|
                @height = (s.fraction * MAXHEIGHT)
            end
            para "height", margin_left: 10
        end
    end

    animate(24) do
        if ((app.width != @width) or (app.height != @height))
            app.resize @width, @height
        end
    end
end
