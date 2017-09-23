Shoes.app width: 300, height: 200 do
  stack do
    flow do
      #button "Button", font: "mono bold 14",width: 200, stroke: red,
      button "Button", font: "Monaco bold 14",width: 200, stroke: red,
          tooltip: "Red mono" do
      end
      button "icon", width: 80, height: 30, icon: "#{DIR}/static/icon-info.png",
          tooltip: "info button" do
      end
    end
    button "Normal", tooltip: "Normal" do
    end
  end
end
