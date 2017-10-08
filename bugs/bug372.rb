Shoes.app(width: 250, height: 100, resizable: false) do
    @p = para
    flow do
        button("enable") do
            app.resizable = true
        end
        button("disable") do
            app.resizable = false
        end
    end
    animate do
        @p.replace "resizable #{app.resizable}"
    end
end
