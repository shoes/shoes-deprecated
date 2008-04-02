Shoes.app :width => 408, :height => 334, :resizable => false do
  background "#eee"
  stack :margin => 4 do
    @vid = video "http://whytheluckystiff.net/o..e/adventure_time.flv"
  end
  para "controls: ",
    link("play")  { @vid.play }, ", ",
    link("pause") { @vid.pause }, ", ",
    link("stop")  { @vid.stop }, ", ",
    link("hide")  { @vid.hide }, ", ",
    link("show")  { @vid.show }, ", ",
    link("+5 sec") { @vid.time += 5000 }
end
