Shoes.app do
    stack do
        @p = para ""
        @sl = slider fraction: 0.5, state: "disabled" do |sl|
            @p.text = "changed : #{sl.fraction}"
        end
        button "switch" do
            @sl.state = @sl.state == "disabled" ? nil : "disabled"
        end
    end
end

