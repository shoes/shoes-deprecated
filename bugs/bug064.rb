Shoes.app(width: 350, height: 200) do
    @x, @y = app.x, app.y
    @p = para
    stack do
        flow do
            tagline "set x: "
            @edx = edit_line
            @edx.text = @x
        end
        flow do
            tagline "set y: "
            @edy = edit_line
            @edy.text = @y
        end
        button("move") do
            @x, @y = @edx.text.to_i, @edy.text.to_i
            app.move @x, @y
        end
    end

    animate do
        @p.replace "Window at [#{app.x}, #{app.y}]"
    end
end
