alert("Issue #60 in action")
ans = confirm("Continue")
if ans
  Shoes.app do
    para "Test"
    button "Quit" do
     exit
    end
  end
end
