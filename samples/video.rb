Shoes.app do
  stack do
    @vid = video "http://whytheluckystiff.net/o..e/adventure_time.flv"
    para "controls: ",
      link("play")  { @vid.play }, ", ",
      link("pause") { @vid.pause }, ", ",
      link("stop")  { @vid.stop }, ", ",
      link("hide")  { @vid.hide }, ", ",
      link("show")  { @vid.show }
  end
end
