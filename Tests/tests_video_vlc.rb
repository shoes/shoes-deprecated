# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'video_vlc.rb'

require 'shoes/videoffi'

Shoes.app title: "Testing Shoes video" do
    
    TestRunner = Test::Unit::UI::Console::TestRunner
    
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
        VideoVlcTest6 => { desc: "Passing vlc options to underlaying libvlc_new() C method",
                           cont_args: {width: 36, height: 21},
                           widget_args: ['AnemicCinema1926marcelDuchampCut.mp4', 
                                          vlc_options: ["--no-xlib", "--no-video-title-show"] ]
                         },                 
        VideoVlcTestAudio => { desc: "audio method + volume, volume= methods",
                               cont_args: {width: 35, height: 20},
                               widget_args: ['indian.m4a', width: 350, height: 20]
                             },                 
         
    }
    
    
    style(Shoes::Para, size: 10)
    
    build_test_gui = -> (t, details, silent) do
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
            ## There could be only one console at a time (we can safely call it many times)
            Shoes.show_console unless silent
            if RUBY_PLATFORM =~ /darwin/
              $stdout = $stderr
            end
            
            out_level = silent ? TestRunner::SILENT : TestRunner::NORMAL
            @test_result = TestRunner.run(t.suite, output_level: out_level)
            
            puts "#{'='*40}\n\n" unless silent
            @sand_box.clear
            @vid = nil
            @sand_box.start { @test_complete = true }
        }
    end
    
    stack margin_left: 10, margin_right: 10 do
        para "Launches Shoes terminal to collect results of Tests", align: "center"
        
        flow do
            @sand_box = stack(height: 50, width: 279) {}
            flow width: 300 do
                stack do
                    flow do
                        para "Silent tests : "
                        @no_output = check checked: true
                    end
                    flow do
                        para "stop tests on failure : "
                        @stop_on_failure = check checked: false
                    end
                end
                @visual = rect 200, 0, 30, 30, 5, fill: white
            end
        end
        
        flow margin: [0,0,0,5] do
            
            button "run ALL Tests" do
                @got_failure = false
                @visual.style(fill: white)
                
                # We have to wait for each test to complete (asynchronous drawing)
                @test_complete = true
                tenum = TESTS.each_with_index
                anm = animate(20) do
                    
                    if @test_complete
                        if @test_result && @test_result.error_occurred?
                            @test_result = nil
                            @visual.style(fill: yellow)
                            anm.stop; anm.remove; anm = nil
                        elsif @test_result && @test_result.failure_occurred?
                            @got_failure = true
                            
                            if @stop_on_failure.checked?
                                @visual.style(fill: red)
                                anm.stop; anm.remove; anm = nil
                            end
                            @test_result = nil
                        else
                            @test_result = nil
                            begin
                                h,i = tenum.next
                                
                                @test_complete = false
                                build_test_gui.call(h[0], h[1], @no_output.checked?)
                                
                            rescue StopIteration
                                @visual.style(fill: @got_failure ? red : green)
                                anm.stop; anm.remove; anm = nil
                            end
                        end
                    end
                end
            end
            
            para "  in order of appearance, top to bottom, click below to show detailed tests"
        end
        
        flow height: 30, margin_bottom: 5 do
            bg = background "#fff".."#eed"
            hover { |s| bg.fill = "#ddd".."#ba9"; s.refresh_slot }
            leave { |s| bg.fill = "#fff".."#eed"; s.refresh_slot }
            click { |s| @tests_slot.toggle }
            
            para "click to see/hide detailed list of tests", margin_left: 50
        end
        
        @tests_slot = stack hidden: true do
            TESTS.each do |t, details|
                flow do
                    button "run Test" do
                        @visual.style(fill: white)
                        # always show output (output_level == NORMAL)
                        build_test_gui.call(t, details, false)
                    end

                    para *(["#{t.to_s} : \n"] << details[:desc])
                end
            end
        end
        
    end
    
    def cont; @cont end
    def vid; @vid end
    VideoVlcTestBase.init self
    
end
