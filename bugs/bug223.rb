
Shoes.app  width: 640, height: 480, title: "Der kleine Sheltie" do
  require 'shoes/videoffi'
  Vlc.load_lib
  @stk = stack do
    @vid = video("/home/ccoupe/Projects/shoes3/Tests/AnemicCinema1926marcelDuchampCut.mp4", autoplay: true)
  end
  @stk.finish {@vid.stop}
end
