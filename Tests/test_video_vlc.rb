# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'

require 'shoes/videoffi'
Vlc.load_lib

require 'video_vlc_01.rb'

Shoes.app title: "Testing Shoes video" do
    
    TESTS = {
        VideoVlcTest0 => { desc: ["Most Basic unique Test : no path/url, no dimensions\n",
                              span("Run this first to ensure that video widget works at its bare minimum",
                              stroke: darkred)], 
                           cont_args: {},
                           widget_args: ['']
                         },
        VideoVlcTest1 => { desc: ["Iinitialisation of video widget without a given path to a media\n",
                            "path= method \nvideo widget ", span("Container", weight: 'bold'), " dimensions given "],
                           cont_args: {width: 30, height: 20},
                           widget_args: ['', bg_color: yellow]
                         },
        VideoVlcTest2 => { desc: ["Iinitialisation of video widget without a given path to a media\n",
                            "path= method \nvideo ", span("Widget", weight: 'bold'), " dimensions given "],
                           cont_args: {},
                           widget_args: ['', width: 25, height: 20]
                         },
        VideoVlcTest3 => { desc: ["Iinitialisation of video widget without a given path to a media\n",
                            "path= method \nvideo ", span("Widget and Container", weight: 'bold'), " dimensions given "],
                           cont_args: {width: 35, height: 20},
                           widget_args: ['', width: 20, height: 20]
                         },
        VideoVlcTest4 => { desc: "no Dimensions provided, relying on video track dimensions",
                           cont_args: {},
                           widget_args: ['AnemicCinema1926marcelDuchampCut.mp4']
                         },                 
        VideoVlcTest5 => { desc: "Widget background color",
                           cont_args: {width: 36, height: 21},
                           widget_args: ['', bg_color: yellow ]
                         },
        VideoVlcTestAudio => { desc: "audio method",
                               cont_args: {width: 35, height: 20},
                               widget_args: ['indian.m4a', width: 350, height: 20]
                             },                 
        VideoVlcTest7 => { desc: "Passing vlc options to underlaying libvlc_new() C method",
                           cont_args: {width: 36, height: 21},
                           widget_args: ['AnemicCinema1926marcelDuchampCut.mp4', 
                                          vlc_options: ["--no-xlib", "--no-video-title-show"] ]
                         },
    }
    
    
    style(Shoes::Para, size: 10)
    
    build_test_gui = -> (t, details) do
        # if we try to draw tests widgets in a non visible area, drawing events are not triggered and
        # Test processing hangs (waits) - start event is not fired -
        # So we create a sand_box at top of Shoes app and scroll back there if necessary to 
        # make drawing events happening 
        app.slot.scroll_top = 0
        
        @sand_box.append do
            @cont = flow( **details[:cont_args] ) do
                @vid = 
                if t == VideoVlcTestAudio
                    audio( *details[:widget_args] )
                else
                    video( *details[:widget_args] )
                end
            end
        end
        
        # waiting for shoes asynchronous drawing events to occur.
        @cont.start {
            #an = animate(10) { |fr|
            #if fr == 2
                #an.stop
                ## There could be only one console at a time (we can safely call it many times)
                #Shoes.show_console
                
                Test::Unit::UI::Console::TestRunner.run(t.suite)
                
                puts "#{'='*40}\n\n"
                @sand_box.clear { para "Next test !" }
                @sand_box.start { @test_complete = true }
                #an.remove; an = nil
            #end
            #}
        }
    end
    
    @top = stack do
        para "\t\tLaunches Shoes terminal to collect results of Tests"
        
        @sand_box = stack(height: 50) {}
        
        flow margin: [0,0,0,10] do
            button "run ALL Tests" do
                
                # We have to wait for each test to complete (asynchronous drawing)
                @test_complete = true
                tenum = TESTS.each_with_index
                anm = animate(20) do
                    if @test_complete
                        @test_complete = false
                        h,i = tenum.next
                        build_test_gui.call(h[0], h[1])
                    
                        if i == TESTS.size-1
                            anm.stop
                            anm.remove; anm = nil
                        end
                    end
                end
            end
            
            para "  in order of appearance, top to bottom"
        end
        
        TESTS.each do |t, details|
            flow do
                button "run Test" do
                    build_test_gui.call(t, details)
                end

                para *(["#{t.to_s} : \n"] << details[:desc])
            end
        end
        
    end
    
    def cont; @cont end
    def vid; @vid end
    VideoVlcTestBase.init self
    
end
