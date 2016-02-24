# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'

require 'shoes/videoffi'
Vlc.load_lib

require 'video_vlc_01.rb'
    
Shoes.app title: "Testing Shoes video" do
    VideoVlcTestBase.init self
    # we must wait for shoes asynchronous drawing events to occur
    @run_test = -> (suite) { Thread.new {Test::Unit::UI::Console::TestRunner.run(suite)} }
    
    style(Shoes::Para, size: 10)
    stack do
        para <<-EOS[0...-1], stroke: darkred
        Launches Shoes console to collect results of Tests, 
        for now, don't close that window, it will exit Shoes ! 
        EOS
        #'
        flow margin: [0,0,0,10] do 
            button "run ALL Tests" do
                @console = Shoes.show_console unless  @console
                Thread.new {
                    [VideoVlcTest1, VideoVlcTest2
                    ].each { |t| Test::Unit::UI::Console::TestRunner.run(t.suite)}
                }
            end
            para "All Tests"
        end
        flow do 
            button "run Test" do
                @console = Shoes.show_console unless  @console
                @run_test.call(VideoVlcTest1.suite)
            end
            para "VideoVlcTest1 : \nTesting initialisation of video widget without a given path to a media"
        end
        flow do 
            button "run Test" do
                @console = Shoes.show_console unless  @console
                @run_test.call(VideoVlcTest2.suite)
            end
            para "VideoVlcTest2 : \nTesting initialisation of video widget with a given path to a movie\n" \
                    "relaying on video track size if no widget or parent canvas dimensions provided"
        end
    end
    
    ### shows alternative to build a suite
    def run_test01
        #my_tests = Test::Unit::TestSuite.new("My Tests")
        #my_tests << VideoVlcTest1.new('test_video_noPath_noDim')
        #my_tests << VideoVlcTest1.new('test_video_noPath_videoWidgetDim')
        #my_tests << VideoVlcTest1.new('test_video_noPath_slotDim')
        #my_tests << VideoVlcTest1.new('test_video_noPath_slotVideoWidgetDim')
        #
        #@run_test.call(my_tests)
    end
    
end
