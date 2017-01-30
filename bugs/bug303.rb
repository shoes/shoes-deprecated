# bug 303 - alert mis-behaves when naked
ask_color "do you see me?"
Shoes.app do
  button "Ask 2" do ask "this is second alert" end
  button "Window 2" do
     window title: "Second Window" do
       button "Confirm 3" do confirm "third alert" end
     end
  end
end
