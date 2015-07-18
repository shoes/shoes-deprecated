Shoes.app title: "Bug 119" do
    button "new" do
        window title: "Deputy", width: 300, height: 200 do
            button("ask") {r = ask "do you ?"; alert "'#{r}'\nGlad to know !"}
            button("confirm") { r = confirm "sure ?"; alert "corroborated ?  #{r}" }
        end 
    end
end
