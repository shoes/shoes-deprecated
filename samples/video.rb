Shoes.app do
  stack do
    @vid = video "http://www.youtube.com/get_video?video_id=LNVYWJOEy9A&t=OEgsToPDskKe6nayFUZBTTHAUtUZixyM&rel=1&border=0"
    para "controls: ",
      link("play")  { @vid.play }, ", ",
      link("pause") { @vid.pause }, ", ",
      link("stop")  { @vid.stop }, ", ",
      link("hide")  { @vid.hide }, ", ",
      link("show")  { @vid.show }
  end
end
