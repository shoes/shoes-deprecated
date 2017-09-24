Shoes.app width: 300, height: 200 do
  stack do
    flow do
      #button "Button", font: "mono bold 14",width: 200, stroke: red,
      button "Button", font: "Menlo Bold 14",width: 200, stroke: red,
          tooltip: "Menlo Bold 14" do
      end
      button "icon", width: 80, height: 30, icon: "#{DIR}/static/icon-info.png",
          tooltip: "info button" do
      end
    end
    button "Normal", tooltip: "Normal" do
    end
    flow do
      button "Curry", font: "Courier New Italic 12",
        tooltip: "Courier New Italic 12" do
      end
      button "Short", font: "Monaco 17", tooltip: "Monaco 17" do
      end
    end
  end
end
