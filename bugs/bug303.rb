# bug 303 - alert mis-behaves when naked
alert "do you see me?"
Shoes.app do
  button "Alert 2" do alert "this is second alert" end
  button "Window 2" do
     window title: "Second Window" do
       button "Alert 3" do alert "third alert" end
     end
  end
end
