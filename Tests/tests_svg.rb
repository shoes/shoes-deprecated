# encoding: UTF-8

Shoes.terminal
$stdout.puts "TERM=#{ENV['TERM']}"
STDOUT.puts "STDOUT"  
$stdout.puts #fails ?
$stdout.puts "tests are ready"
require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'svg.rb'


Shoes.app title: "Testing Shoes::Svg", resizable: false do
    
    TestRunner = Test::Unit::UI::Console::TestRunner
    
    # width, height : 190x110, subid = '#xml_rectangle', 180x100, offsetx = 5, offsety = 5
    XML = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <svg xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:cc="http://creativecommons.org/ns#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg" version="1.1" id="svg2" viewBox="0 0 190 110" height="110" width="190">
      <defs id="defs4" />
      <metadata id="metadata7">
        <rdf:RDF>
          <cc:Work rdf:about="">
            <dc:format>image/svg+xml</dc:format>
            <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
            <dc:title></dc:title>
          </cc:Work>
        </rdf:RDF>
      </metadata>
      <g transform="translate(0,-942.36216)" id="xml_rectangle">
        <rect y="949.33234" x="6.9701514" height="96.0597" width="176.05969" id="rectangle001" style="opacity:0.98000004;fill:#ff7131;fill-opacity:1;fill-rule:nonzero;stroke:#000000;stroke-width:3.94030356;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1" />
      </g>
    </svg>'
    
    # width, height : 1000x600, subid = '#test_rectangle', 980x550, offsetx = 5, offsety = 45
    FILE = "simpletest.svg"
    
    TESTS = {
        SvgTest0 => { desc: ["Basic Test : simple xml svg, no dimensions\n"], 
                      cont_args: {},
                      widget_args: [XML]
                    },
        SvgTest1 => { desc: ["Basic Test : file svg, no dimensions\n",
                             "Relying on Shoes app size and svg size (svg is larger than app here)\n",
                             "So keep 'resizable: false' for the app style !"], 
                      cont_args: {},
                      widget_args: [FILE]
                    },
        SvgTest2 => { desc: ["Simple xml svg, parent dimensions\n"], 
                      cont_args: {width: 30, height: 20},
                      widget_args: [XML]
                    },
        SvgTest3 => { desc: ["Simple xml svg, svg method dimensions\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 30, height: 20]
                    },
        SvgTest4 => { desc: ["Simple xml svg, parent and svg method dimensions\n"], 
                      cont_args: {width: 30, height: 25},
                      widget_args: [XML, width: 25, height: 20]
                    },
        SvgTest5 => { desc: ["Xml svg, Landscape, aspect ratio = false\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 48, height: 32, aspect: false]
                    },
        SvgTest5B => { desc: ["Xml svg, Landscape, aspect ratio = 0.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 48, height: 32, aspect: 0.5]
                    },
        SvgTest5C => { desc: ["Xml svg, Landscape, aspect ratio = 1.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 48, height: 32, aspect: 1.5]
                    },
        SvgTest6 => { desc: ["Xml svg, Square, aspect ratio = 0.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 48, height: 48, aspect: 0.5]
                    },
        SvgTest7 => { desc: ["Xml svg, Square, aspect ratio = 1.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 48, height: 48, aspect: 1.5]
                    },
        SvgTest8 => { desc: ["Xml, group id\n"], 
                      cont_args: {},
                      widget_args: [XML, group: "#xml_rectangle"]
                    },           
        SvgTest9 => { desc: ["File, group id\n"], 
                      cont_args: {},
                      widget_args: [FILE, group: "#test_rectangle"]
                    },
        SvgTest10 => { desc: ["Xml svg, Portrait, aspect ratio = true\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 32, height: 48, aspect: true]
                    },
        SvgTest11 => { desc: ["Xml svg, Portrait, aspect ratio = false\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 32, height: 48, aspect: false]
                    },
        SvgTest12 => { desc: ["Xml svg, Portrait, aspect ratio = 0.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 32, height: 48, aspect: 0.5]
                    },
        SvgTest13 => { desc: ["Xml svg, Portrait, aspect ratio = 1.5\n"], 
                      cont_args: {},
                      widget_args: [XML, width: 32, height: 48, aspect: 1.5]
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
                @svg_obj = svg( *details[:widget_args] )
            end
        end
        
        # waiting for shoes asynchronous drawing events to occur.
        @cont.start {
            ## There could be only one console at a time (we can safely call it many times)
            Shoes.terminal unless silent
           
            out_level = silent ? TestRunner::SILENT : TestRunner::NORMAL
            @test_result = TestRunner.run(t.suite, {output_level: out_level,
				output: $stderr})
            
            puts "#{'='*40}\n\n" unless silent
            @sand_box.clear
            @svg_obj = nil
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
                        #@no_output = check checked: true
                        @no_output = check checked: false
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
    def svg_obj; @svg_obj end
    SvgTestBase.init self
    
end
