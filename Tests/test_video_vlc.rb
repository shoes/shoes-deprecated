# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'

require 'shoes/videoffi'
Vlc.load_lib

require 'video_vlc_01.rb'
    
Shoes.app title: "Testing Shoes video" do
    VideoVlcTestBase.init self
    # we must wait for shoes asynchronous drawing events to occur
    @run_test = -> (suite) { Thread.new {Test::Unit::UI::Console::TestRunner.run(suite)}; puts }
    
    style(Shoes::Para, size: 10)
    stack do
        para <<-EOS[0...-1], stroke: darkred
        Launches Shoes console to collect results of Tests, 
        for now, don't close that window, it will exit Shoes ! 
        EOS
        #'
        flow margin: [0,0,0,10] do 
            button "run ALL Tests" do
                # There could be only one console at a time (we safely can call it many times)
                Shoes.show_console
                Thread.new {
                    [VideoVlcTest1, VideoVlcTest2, VideoVlcTest3, VideoVlcTest4
                    ].each { |t| Test::Unit::UI::Console::TestRunner.run(t.suite); puts}
                }
            end
            para "All Tests"
        end
        flow do 
            button "run Test" do
                Shoes.show_console
                @run_test.call(VideoVlcTest1.suite)
            end
            para "VideoVlcTest1 : \nTesting initialisation of video widget without a given path to a media\n" \
                    "Testing procedure to wait for Drawing Area to be ready for vlc to use it"
        end
        flow do 
            button "run Test" do
                Shoes.show_console
                @run_test.call(VideoVlcTest2.suite)
            end
            para "VideoVlcTest2 : \nTesting initialisation of video widget with a given path to a movie\n" \
                    "relaying on video track size if no widget or parent canvas dimensions provided"
        end
        
        flow do 
            button "run Test" do
                Shoes.show_console
                @run_test.call(VideoVlcTest3.suite)
            end
            para "VideoVlcTest3 : \nTesting audio facility"
        end
        
        flow do 
            button "run Test" do
                Shoes.show_console
                @run_test.call(VideoVlcTest4.suite)
            end
            para "VideoVlcTest4 : \nTesting media loading"
        end
    end
    
    ### shows alternative to build a suite
    #def run_test01
    #    my_tests = Test::Unit::TestSuite.new("My Tests")
    #    my_tests << VideoVlcTest1.new('test_video_noPath_noDim')
    #    my_tests << VideoVlcTest1.new('test_video_noPath_videoWidgetDim')
    #    my_tests << VideoVlcTest1.new('test_video_noPath_slotDim')
    #    my_tests << VideoVlcTest1.new('test_video_noPath_slotVideoWidgetDim')
    #    
    #    @run_test.call(my_tests)
    #end
    
end
