# encoding: UTF-8

class VideoVlcTestBase < Test::Unit::TestCase
    class << self
        def init(app)
            @@app = app
        end
        def startup
            @@mediav = "AnemicCinema1926marcelDuchampCut.mp4"
            @@mediaa = "indian.m4a"
            # a fake stream : not relying on internet's mood swings for the test
            @@mediast = "file://#{File.expand_path('./AnemicCinema1926marcelDuchampCut.mp4')}"
        end
        
        def shutdown
            @@mediav = nil
            @@mediaa = nil
            @@mediast = nil
        end
    end

    def setup; end
    def teardown; end
end

class VideoVlcTest0 < VideoVlcTestBase

    description "no dimensions provided, no path/url, nothing to draw"
    def test_video_noPath_noDim
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        vidc = @@app.vid.instance_variable_get(:@video)
        assert_true vidc.drawable_ready?
        
        vlci = @@app.vid.instance_variable_get(:@vlci)
        assert_not_nil vlci
        assert_instance_of Fiddle::Pointer, vlci
        assert_false vlci.null?
        
        assert_equal 0, @@app.vid.width
    end
end

class VideoVlcTest1 < VideoVlcTestBase
    
    def test_slotDim_noPath
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 30, @@app.vid.width
    end
    
    def test_slotDim_movie
        @@app.vid.path = @@mediav
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 30, @@app.vid.width
        
        assert_equal @@mediav, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_video_track
        assert_equal 640, @@app.vid.video_track_width
        assert_equal 480, @@app.vid.video_track_height
        assert_true @@app.vid.have_audio_track
    end
    
    def test_slotDim_audio
        @@app.vid.path = @@mediaa
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 30, @@app.vid.width
        
        assert_equal @@mediaa, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_audio_track
        assert_nil @@app.vid.have_video_track
        assert_nil @@app.vid.video_track_width
        assert_nil @@app.vid.video_track_height
    end
    
    def test_slotDim_stream
        @@app.vid.path = @@mediast
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 30, @@app.vid.width
        
        assert_equal @@mediast, @@app.vid.path
        assert_true @@app.vid.loaded
    end
end

class VideoVlcTest2 < VideoVlcTestBase
    
    def test_widgetDim_noPath
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 25, @@app.vid.width
    end
    
    def test_widgetDim_movie
        @@app.vid.path = @@mediav
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 25, @@app.vid.width
        
        assert_equal @@mediav, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_video_track
        assert_equal 640, @@app.vid.video_track_width
        assert_equal 480, @@app.vid.video_track_height
        assert_true @@app.vid.have_audio_track
    end
    
    def test_widgetDim_audio
        @@app.vid.path = @@mediaa
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 25, @@app.vid.width
        
        assert_equal @@mediaa, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_audio_track
        assert_nil @@app.vid.have_video_track
        assert_nil @@app.vid.video_track_width
        assert_nil @@app.vid.video_track_height
    end
    
    def test_widgetDim_stream
        @@app.vid.path = @@mediast
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 25, @@app.vid.width
        
        assert_equal @@mediast, @@app.vid.path
        assert_true @@app.vid.loaded
    end
end

class VideoVlcTest3 < VideoVlcTestBase
    
    def test_slotDim_widgetDim_noPath
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 20, @@app.vid.width
    end
    
    def test_slotDim_widgetDim_movie
        @@app.vid.path = @@mediav
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 20, @@app.vid.width
        
        assert_equal @@mediav, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_video_track
        assert_equal 640, @@app.vid.video_track_width
        assert_equal 480, @@app.vid.video_track_height
        assert_true @@app.vid.have_audio_track
    end
    
    def test_slotDim_widgetDim_audio
        @@app.vid.path = @@mediaa
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 20, @@app.vid.width
        
        assert_equal @@mediaa, @@app.vid.path
        assert_true @@app.vid.loaded
        assert_true @@app.vid.have_audio_track
        assert_nil @@app.vid.have_video_track
        assert_nil @@app.vid.video_track_width
        assert_nil @@app.vid.video_track_height
    end
    
    def test_slotDim_widgetDim_stream
        @@app.vid.path = @@mediast
        
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 20, @@app.vid.width
        
        assert_equal @@mediast, @@app.vid.path
        assert_true @@app.vid.loaded
    end
end

class VideoVlcTest4  < VideoVlcTestBase
    def test_videoTrackDim
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        
        assert_equal 640, @@app.vid.width
        assert_equal 480, @@app.vid.height
    end
end

class VideoVlcTest5  < VideoVlcTestBase
    def test_backgroundColor
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        assert_equal 36, @@app.vid.width
        
        assert_equal 255, @@app.vid.style[:bg_color].red
        assert_equal 255, @@app.vid.style[:bg_color].green
        assert_equal 0, @@app.vid.style[:bg_color].blue
    end
end

class VideoVlcTest6  < VideoVlcTestBase
    def test_libvlcOptions
        assert_not_nil @@app.vid
        vlci = @@app.vid.instance_variable_get(:@vlci)
        assert_not_nil vlci
        assert_instance_of Fiddle::Pointer, vlci
        assert_false vlci.null?
        assert_false @@app.vid.style.include?(:vlc_options)
    end
end

class VideoVlcTestAudio  < VideoVlcTestBase
    def test_audioMethod
        assert_not_nil @@app.vid
        assert_instance_of Shoes::VideoVlc, @@app.vid
        
        assert_equal 0, @@app.vid.width
        assert_true @@app.vid.style[:hidden]
        
        assert_equal 85, @@app.vid.volume
        @@app.vid.volume = 72
        assert_equal 72, @@app.vid.volume
    end
end


