alert("Issue #60 in action")
ans = confirm("Continue")
if ans
  Shoes.app do
    para "Test"
    button "Quit" do
     Shoes.quit
    end
  end
end
