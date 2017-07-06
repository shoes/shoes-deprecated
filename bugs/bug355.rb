Shoes.app(title: " v1.0", width: 300, height: 500, resizable: true ) do
    background green
    #@main = stack left: 0.05, top: 0.15, width: 0.9, height: 0.8, scroll: true do
    @main = stack margin_left: 10, margin_top: 10, margin_bottom: 20, margin_right: 10+gutter do
        background beige
        100.times do |i|
            para "#{i+1} times"
        end
    end
end
