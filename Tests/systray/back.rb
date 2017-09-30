Shoes.app do
  stack do
    para "Generate 10 systray calls - 30 seconds appart."
    para "Press button to start and cover the app window with another app"
    ctr = 0;
    button "Notify" do
      @ev = every 30 do
	      ctr += 1
	      icp = ''
	      if ctr % 3 != 0
	        icp = "#{DIR}/static/shoes-icon.png"
	      else
	        icp = "#{DIR}/static/shoes-icon-red.png"
	      end
	      systray title: "Shoes Notify Test", message: "message ##{ctr}", icon: icp
	      @ev.stop if ctr >=  10
	  end
    end
  end 
end
