Shoes.app do
  stack do
    para "Press button and look in your systems notication area"
    ctr = 0;
    button "Notify" do
      ctr += 1
      systray title: "Notify Test", message: "message ##{ctr}", icon: "#{DIR}/static/shoes-icon.png"
    end
  end 
end
