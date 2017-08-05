Shoes.app do
  stack do
    para "Press button and look in your systems notication area"
    ctr = 0;
    button "Notify" do
      ctr += 1
      icp = ''
      if ctr % 3 != 0
        icp = "#{DIR}/static/shoes-icon.png"
      else
        icp = "#{DIR}/static/shoes-icon-red.png"
      end
      systray title: "Notify Test", message: "message ##{ctr}", icon: icp
    end
  end 
end
