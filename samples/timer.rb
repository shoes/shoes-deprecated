label, time = nil, Time.now
Shoes.app :height => 150, :width => 250 do
  background rgb(240, 250, 208)
  stack :margin => 10 do
    button "Start" do
      time = Time.now
      label.replace "Stop watch started at #{time}"
    end
    button "Stop" do
      label.replace "Stopped, #{Time.now - time} seconds elapsed."
    end
    label = text "Press start to begin timing."
  end
end
