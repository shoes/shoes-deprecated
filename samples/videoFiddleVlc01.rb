# encoding: UTF-8

class Shoes::Knob < Shoes::Widget
    attr_reader :canvas, :fraction, :tint
    attr_accessor :range, :tick, :active

    def initialize(driven, opts = {})
        
        @driven = driven
        
        @active = true
        @range = opts[:range] || 0..100
        @fraction = opts[:fraction] || 80
        @click_proc = opts[:click]
        
        @tint = opts[:color] || lawngreen
        @tick = opts[:tick] || 10
        @size = opts[:size] || 1
        @padding = opts[:padding] || 0
        self.width = (50*@size).to_i + @padding     
        @cx, @cy = self.left + (25*@size) + @padding, self.top + (25*@size)
        
        @canvas = flow :height => (50*@size).to_i, click: @click_proc,
                    hover: proc {|s| s.cursor = :hand}, leave: proc {|s| s.cursor = :arrow} do
            
            fill black
            stroke white
            oval :left => @cx - (20*@size), :top => @cy - (20*@size), :radius => (20*@size)
            
            nofill
            oval :left => @cx - (12*@size), :top => @cy - (12*@size), :radius => (12*@size)
            
            strokewidth 1
            (ticks + 1).times do |i|
                radial_line 225 + ((270.0 / ticks) * i), (12*@size)..(15*@size)
            end
            
            strokewidth 2
            stroke @tint
            fill @tint
            @ovl = oval :left => @cx - (3*@size), :top => @cy - (3*@size), :radius => (3*@size)
            
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
        
    end
    
    def tint=(color)
        @tint = color
        @needle.remove
        @ovl.remove
        
        @canvas.append do
            strokewidth 2
            stroke @tint
            fill @tint
            @ovl = oval :left => @cx - (3*@size), :top => @cy - (3*@size), :radius => (3*@size)
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
    end
    
    def ticks; @range.end / @tick end

    def radial_line(deg, r)
        pos = ((deg / 360.0) * (2.0 * Math::PI)) - (Math::PI / 2.0)
        line (Math.cos(pos) * r.begin) + @cx, (Math.sin(pos) * r.begin) + @cy,
             (Math.cos(pos) * r.end) + @cx, (Math.sin(pos) * r.end) + @cy
    end

    def fraction= pos
        @fraction = pos
        @needle.remove
        
        @canvas.append do
            strokewidth 2
            stroke @tint
            fill @tint
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
    end
    
    # Calling bound method of the driven element.
    # args is an array of the arguments given to the method
    # if method has more than one argument, the driven argument is the one, in the array, marqued as "driver",
    #       ie: for shape.displace(x,y) --> tweak(left, ["driver", 5]) 
    #       displace.x beeing driven by knob, capturing mouse's left coordinates
    # other arguments are given default values 
    def tweak(origin_left, args = nil)
        @released = false
        origin_pos = @fraction
        
        self.app.motion do |lf,t|
            pos =(origin_pos - (origin_left - lf))
            if @active and not @released and @range.cover?(pos)
                self.fraction= pos
                if args
                    @driven.call( *args.map { |v| v = @fraction if v == "driver" } )
                else
                    @driven.call @fraction
                end
            end
        end
        self.app.release {|b,l,t| @released = true}
    end
end


require 'shoes/videoffi'
Vlc.load_lib 
#Vlc.load_lib path: '/home/xy/NBWorkspace/vlc/lib/.libs/libvlc.so.5.5.0', plugin_path: "/home/xy/NBWorkspace/vlc/modules" 

Shoes.app width: 625, height: 580, resizable: true do
    LinkStyleStopped = [Shoes::Link, stroke: black, underline: "none"]
    LinkStyleStoppeddHover = [Shoes::LinkHover, stroke: darkred, underline: "none"]
    style(*LinkStyleStopped); style(*LinkStyleStoppeddHover)
    CtrlsText = "    ",
                link("play")   { play_media }    , "   ",
                link("pause")  { toggle_media }  , "   ",
                link("stop")   { stop_media }    , "      ",
                link("<<")     { @svlc.previous_media } , "   ",
                link(">>")     { @svlc.next_media } , "      ",
                link("+5 sec") { @svlc.time += 5000 } , "   ",
                link("-5 sec") { @svlc.time -= 5000 } , "   "
    start_vol = 75
    
    stack do
        @info = para "", margin_left: 25, size: 11
        @cont = flow do   #  width: 600, height: 400 
        @svlc = video "", margin_left: 25, autoplay: true, #width: 600, height: 400,
                          volume: start_vol, bg_color: rgb(20,20,20)
        end
        
        @timeline = progress width: 1.0, height: 10, margin: [25,0,0,0]
        @cont.start { |slot| @timeline.width = slot.width-25 }
        
        @controls = flow margin: [0,10,0,0], height: 47 do
            # slot needs a height, here, for background to work correctly
            # an alternative is to put @controls slot following (gui) code into a @controls.start {} block
            @bckgrd = background rgb(242,241,240)
            
            @ctrls = para CtrlsText
            para "    autoplay :" 
            @chk = check checked: @svlc.autoplay, margin: [0,0,10,0] do |c| 
                @svlc.autoplay = c.checked?
            end
            
            driven = @svlc.method(:volume=)
            @vol_knob = knob driven, fraction: start_vol, padding: 20, size: 0.75, color: red,
                            click: proc { |but,left,t| @vol_knob.tweak(left) }
        end
        
        flow margin: [100,0,0,0] do
            button("half size") { set_video_dim 0.5 }
            button("real size") { set_video_dim 1 }
            button("double size") { set_video_dim 2 }
            para "  show video   "
            check checked: true do |c|
                c.checked? ? @svlc.show : @svlc.hide
            end
        end
                
        @url_slot = flow hidden: true, margin_bottom: 5 do
            para  "Enter the url of the media you would like to be entertained with : "
            @url_edit = edit_line "", width: 450
            
            button "Play" do
                unless @url_edit.text.nil? or @url_edit.text.empty?
                    @anim.stop
                    @svlc.path = @url_edit.text
                    @info.text = ""
                    @url_edit.text = ""
                    set_controls
                    @anim.start
                end
                
                @url_slot.hide
            end
            button("cancel") { @url_slot.hide }
        end
        
        flow margin: [10,0,0,5] do
            button "Open file" do
                @anim.stop
                file = ask_open_file
                unless file.nil?
                    @svlc.stop
                    @svlc.path = file
                    @info.text = ""
                    @svlc.autoplay ? set_controls : reset_controls
                end
                @anim.start
            end
            
            button "Open stream" do
                @url_slot.show
                @url_edit.focus
                app.slot.scroll_top = app.slot.scroll_max
            end
            
            button "Quit", margin_left: 50 do; exit end;
        end
        
    end
    
    def set_video_dim(dim)
        @svlc.style(width: (@svlc.video_track_width.to_f*dim).to_i, 
                    height: (@svlc.video_track_height.to_f*dim).to_i)
        @timeline.width = (@svlc.video_track_width.to_f*dim).to_i
    end
    
    def set_controls(color=gray)
        @bckgrd.fill = color
        @controls.style(Shoes::LinkHover, stroke: black, underline: "none")
        @controls.style(Shoes::Link, stroke: lawngreen, underline: "none")
        @ctrls.text = CtrlsText # won't refresh until mouse hovering, otherwise 
        @vol_knob.tint = lawngreen
    end
    
    def reset_controls(color=rgb(242,241,240))
        @bckgrd.fill = color
        @controls.style(*LinkStyleStoppeddHover)
        @controls.style(*LinkStyleStopped)
        @ctrls.text = CtrlsText
        @vol_knob.tint = red
    end
    
    def play_media
        @svlc.play
        set_controls 
        @anim.start
    end
    
    def toggle_media
        @svlc.playing? ? reset_controls(rgb(170,170,170)) : set_controls
        @svlc.pause
        info "width = #{@svlc.width}, height = #{@svlc.height}, left = #{@svlc.left}, top = #{@svlc.top}"
    end
    
    def stop_media
        @svlc.stop
        reset_controls
        @anim.stop
    end
    
    def s2hms(seconds)
        return "0s" if seconds <= 0
        minutes = seconds/60
        seconds -= minutes*60
        hours = minutes/60
        minutes -= hours * 60
        "" + (hours > 0 ? "#{hours}h " : "") + (minutes > 0 ? "#{minutes}m " : "") + "#{seconds}s"
    end
    
    set_controls if @svlc.loaded && @svlc.autoplay
    
    @anim = animate(5) do |fr|
        now = @svlc.time/1000
        total = @svlc.length/1000
        
        @timeline.width = @svlc.have_video_track ? @svlc.width : app.width-25
        @timeline.fraction = @svlc.position.round(2)
        @info.text = strong("#{File.basename(@svlc.path)}"), "  playing  #{s2hms(now)} / #{s2hms(total)}"
    end
end

