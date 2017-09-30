Shoes.app do
  stack do
    para "Click the button to reduce opacity to %90"
    button "reduce" do
     app.opacity =  0.90
     para "Opacity is #{app.opacity}\n"
    end
    button "normal" do
     app.opacity = 1.0
     para "normal\n"
    end
  end
end
