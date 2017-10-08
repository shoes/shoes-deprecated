Shoes.app(width: 350, height: 200) do
    @x, @y = app.left, app.top
    @p = para
    stack do
        flow do
            tagline "set left: "
            @edx = edit_line
            @edx.text = @x
        end
        flow do
            tagline "set top: "
            @edy = edit_line
            @edy.text = @y
        end
        button("move") do
            @x, @y = @edx.text.to_i, @edy.text.to_i
            app.move @x, @y
            @p.replace "Window at [#{app.left}, #{app.top}]"
        end
    end

    start do
        @p.replace "Window at [#{@x}, #{@y}]"
    end
end
