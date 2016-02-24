# encoding: UTF-8

class VideoVlcTestBase < Test::Unit::TestCase
    
    def self.init(app)
        @@app = app
    end
       
    def setup
        @started = false
    end
    
    def teardown
        @cont.remove
        @cont = nil
        @vid = nil
    end  
end

class VideoVlcTest1 < VideoVlcTestBase
    
    description "no dimensions provided, no path/url, nothing to draw"
    def test_video_noPath_noDim
        @cont = @@app.flow do 
            @vid = Shoes::VideoVlc.new( @@app, '')
        end
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 0, @vid.width
    end
    
    def test_video_noPath_slotDim
        @cont = @@app.flow width: 300, height: 200  do 
            @vid = Shoes::VideoVlc.new( @@app, '')
        end.start { @started = true }
        
        while not @started do; end
        
        assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 300, @vid.width
    end
     
    def test_video_noPath_videoWidgetDim
        @cont = @@app.flow do 
            @vid = Shoes::VideoVlc.new( @@app, '', width: 250, height: 200 )
        end.start { @started = true }
        
        ## TODO start event is not launched
        ## related to @cont not having a height, hence beeing split, internally in C, in two canvases ... 
        ## see start method enhancement at github https://github.com/passenger94/shoes3/blob/338a3d40421efe37b6fe8b6b4578565d0afbc483/shoes/canvas.c#L1821
        sleep 0.1 
        #puts "@started = #{@started }" ; puts "@cont : #{@cont.style.inspect}"
        #while not @started do; end
        
        #assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 250, @vid.width
    end
     
    def test_video_noPath_slotVideoWidgetDim
        @cont = @@app.flow width: 300, height: 200 do 
            @vid = Shoes::VideoVlc.new( @@app, '', {width: 250, height: 200} )
        end.start { @started = true }
        
        while not @started do; end
        
        assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 250, @vid.width
    end
end

class VideoVlcTest2 < VideoVlcTestBase

    def setup
        super
        @movie = "AnemicCinema1926marcelDuchampCut.mp4"
    end
    
    description "no dimensions provided, relying on video track size"
    def test_video_path_noDim
        @cont = @@app.flow do 
            @vid = Shoes::VideoVlc.new( @@app, @movie )
        end.start { @started = true }
        
        sleep 0.1
        #while not @started do; end
        
        #assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 640, @vid.width
        assert_equal 480, @vid.height
    end
    
    def test_video_path_slotDim
        @cont = @@app.flow width: 300, height: 200  do 
            @vid = Shoes::VideoVlc.new( @@app, @movie )
        end.start { @started = true }
        
        while not @started do; end
        
        assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 300, @vid.width
    end
     
    def test_video_path_videoWidgetDim
        @cont = @@app.flow do 
            @vid = Shoes::VideoVlc.new( @@app, @movie, width: 250, height: 200 )
        end.start { @started = true }
        
        sleep 0.1 
        #while not @started do; end
        
        #assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 250, @vid.width
    end
     
    def test_video_path_slotVideoWidgetDim
        @cont = @@app.flow width: 300, height: 200 do 
            @vid = Shoes::VideoVlc.new( @@app, @movie, width: 250, height: 200 )
        end.start { @started = true }
        
        while not @started do; end
        
        assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 250, @vid.width
    end
end

class VideoVlcTest3 < VideoVlcTestBase
    def setup
        super
        @audio = "indian.m4a"
    end
    
    def test_audio
        @cont = @@app.flow width: 300, height: 200 do 
            @vid = Shoes::VideoVlc.new( @@app, @audio, width: 0, height: 0, hidden: true )
        end.start { @started = true }
        
        while not @started do; end
        
        assert_true @cont.style[:started]
        assert_instance_of Shoes::VideoVlc, @vid
        assert_equal 0, @vid.width
        assert_true @vid.style[:hidden]
    end
    
end



