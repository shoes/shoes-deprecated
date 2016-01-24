require 'fiddle'
require 'fiddle/import'

module Vlc
	extend Fiddle::Importer
    
    # Fiddle's default 'extern' stores all methods into local variable '@func_map', that makes difficult to 'include Vlc'.
    # So override it and replace '@func_map' into 'VLC_FUNCTIONS_MAP'.
    # Ref.: /lib/ruby/2.0.0/fiddle/import.rb
    # Borrowed from https://github.com/vaiorabbit/ruby-opengl
    VLC_FUNCTIONS_MAP = {}
    def self.extern(signature, *opts)
        symname, ctype, argtype = parse_signature(signature, @type_alias)
        opt = parse_bind_options(opts)
        f = import_function(symname, ctype, argtype, opt[:call_type])
        name = symname.gsub(/@.+/,'')
        VLC_FUNCTIONS_MAP[name] = f
        begin
          /^(.+?):(\d+)/ =~ caller.first
          file, line = $1, $2.to_i
        rescue
          file, line = __FILE__, __LINE__+3
        end
        args_str="*args"
        module_eval(<<-EOS, file, line)
          def #{name}(*args, &block)
            VLC_FUNCTIONS_MAP['#{name}'].call(*args,&block)
          end
        EOS
        module_function(name)
        f
    end
    
    
    # libvlc_track_type_t
    LIBVLC_TRACK_UNKNOWN = -1	
    LIBVLC_TRACK_AUDIO   = 0
    LIBVLC_TRACK_VIDEO 	 = 1
    LIBVLC_TRACK_TEXT    = 2
    
    Audio_track = struct [
        'unsigned int i_channels',
        'unsigned int i_rate'
    ]
    
    Video_track = struct [
        'unsigned int i_height',
        'unsigned int i_width',
        'unsigned int i_sar_num',
        'unsigned int i_sar_den',
        'unsigned int i_frame_rate_num',
        'unsigned int i_frame_rate_den'
    ]
    
    Subtitle_track = struct ['char *psz_encoding']
    
#    MT_union = union [
#        'libvlc_audio_track_t *audio',
#        'libvlc_video_track_t *video',
#        'libvlc_subtitle_track_t *subtitle'
#    ]
#    MT_union_alloc = Fiddle::Pointer.malloc(MT_union.size)
    MT_union = union [
        'Audio_track *audio',
        'Video_track *video',
        'Subtitle_track *subtitle'
    ]
    
    Media_track = struct [
        'unsigned int i_codec',
        'unsigned int i_original_fourcc',
        'int i_id',
        'int i_type', # libvlc_track_type_t             ### via regular c lib, vlc call :
                                                        # size of track : 56
        # Codec specific                                # size of track->i_codec : 4
        'int i_profile',                                # size of track->i_original_fourcc : 4
        'int i_level',                                  # size of track->i_id : 4
                                                        # size of track->i_type : 4
        # union inside struct ??                        # size of track->i_profile : 4
        # using an aggregate of same bytes length       # size of track->i_level : 4
        'char kind[8]', # 8 bytes                       # size of track->video : 8 (union)
                                                        # size of track->i_bitrate : 4
        'unsigned int i_bitrate',                       # size of track->psz_language : 8
        'char *psz_language',                           # size of track->psz_description : 8
        'char *psz_description' 
    ]
    
    @@vlc_import_done = false
    def self.import_symbols    
        typealias "uint32_t", "unsigned int"
        typealias "libvlc_time_t", "long long"
        typealias "libvlc_instance_t", "libvlc_instance_t"
        typealias "libvlc_media_t", "libvlc_media_t"
        typealias "libvlc_track_type_t", "int"
        typealias "libvlc_media_player_t", "libvlc_media_player_t"


        extern 'libvlc_instance_t * libvlc_new( int , const char* )'
        ### Media
        extern 'libvlc_media_t * libvlc_media_new_path(libvlc_instance_t*, const char* )'
        extern 'void libvlc_media_parse( libvlc_media_t* )'
        extern 'unsigned int libvlc_media_tracks_get( libvlc_media_t*, libvlc_media_track_t*** )'
        extern 'void libvlc_media_tracks_release( libvlc_media_track_t**, unsigned int )'
        extern 'void libvlc_media_release( libvlc_media_t* )'

        ### Media player
        extern 'libvlc_media_player_t* libvlc_media_player_new( libvlc_instance_t* )'
        extern 'void libvlc_media_player_set_media( libvlc_media_player_t*, libvlc_media_t* )'
        extern 'int libvlc_media_player_play( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_stop( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_pause( libvlc_media_player_t* )'
        extern 'uint32_t libvlc_media_player_get_xwindow( libvlc_media_player_t*)'
        extern 'void libvlc_media_player_set_xwindow( libvlc_media_player_t*, uint32_t)'
        extern 'int libvlc_media_player_is_playing( libvlc_media_player_t*)'
        extern 'libvlc_time_t libvlc_media_player_get_time( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_time(libvlc_media_player_t*, libvlc_time_t )'
        extern 'libvlc_time_t libvlc_media_player_get_length( libvlc_media_player_t* )'
        extern 'float libvlc_media_player_get_position( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_position( libvlc_media_player_t*, float )'
        
        ### Audio controls
        extern 'int libvlc_audio_get_volume( libvlc_media_player_t* )'
        extern 'int libvlc_audio_set_volume( libvlc_media_player_t*, int )'
        
        
        @@vlc_import_done = true
    end
    
    # Load native library.
    def self.load_lib(path = nil)
        if path
            @vlc_lib = path
        else
            case RUBY_PLATFORM
    #        when /win|mswin|msys|mingw|cygwin/     
    #            lib = 'libvlc.dll'                 
    #            @vlc_lib = "C:\Program Files\VideoLAN\VLC\#{lib}"
    #        when /darwin/                          
    #            lib = 'libvlc.dylib'               
    #            @vlc_lib = "/Applications/VLC.app/Contents/MacOS/lib/#{lib}"                      
            when /linux/    
                @vlc_lib = 'libvlc.so'
            else
                raise "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
            end
        end
        
        begin
            dlload @vlc_lib
        rescue => e
            raise "Sorry, No Video support !\n unable to find libvlc :  #{@vlc_lib}"
        end
        
        import_symbols() unless @@vlc_import_done
        
        # just in case ... other functions in Fiddle::Importer are using it
        # The variable is declared in 'dlload'
        @func_map = VLC_FUNCTIONS_MAP
    end

end


class Object
    def unfold(template)
        to_s.unpack(template)[0]
    end
end


class Shoes::VideoVlc
    include Vlc
    
    attr_accessor :autoplay
    attr_reader :path, :player, :width, :height, :loaded
    
    def initialize(app, path, attr=nil, audio=false)
        @path = path
        attr ||= {}
        @autoplay = (attr[:autoplay]) || false
        
        # what should we do with "--no-xlib"/XInitThreads(), seems controversial ...
        # Do we need threaded xlib in shoes/vlc ?
        @vlci = libvlc_new(0, nil)
        
        @player = Vlc::libvlc_media_player_new(@vlci)
        
        @loaded = load_media @path
        
        if audio
            attr[:width] = attr[:height] = 0
            attr[:hidden] = true
        end    
        @video = app.video2 attr
        
        
        @video.parent.start { 
            # Connect the video rendering to a prepared custom Drawing Area.
            drID = @video.drawable  # xlib window / HWND / NSView  id
            if libvlc_media_player_get_xwindow(@player) == 0
                libvlc_media_player_set_xwindow(@player, drID)
                # libvlc_media_player_set_hwnd       on Windows
                # libvlc_media_player_set_nsobject   on osx
            end
            
            play if @loaded && @autoplay
        } 
    end
    
    # would like to get info on tracks, notably width and height of video tracks
    # not mandatory !
    def load_media(path)
        @media = Vlc::libvlc_media_new_path(@vlci, path)
        if @media.null? or @path == ""
            return nil
        else
            libvlc_media_player_set_media(@player, @media)
            libvlc_media_release(@media)
        end
        
        Vlc::libvlc_media_parse(@media)
        
        # allocating space for 5 tracks [ 10, more ? ]
#        tracks_alloc = Fiddle::Pointer[' ' * Vlc::Media_track.size * 5]
        tracks_alloc = Fiddle::Pointer.malloc(Vlc::Media_track.size * 5)
        n_tracks = Vlc::libvlc_media_tracks_get( @media, tracks_alloc.ref )
        
        (0...n_tracks).each do |i|
            track = Vlc::Media_track.new(tracks_alloc.to_i + (Vlc::Media_track.size*i))
            
            puts "      i_type : #{track.i_type.unfold('I!')}"
            if track.i_type.to_s.unpack('i')[0] == Vlc::LIBVLC_TRACK_VIDEO
                puts "      VIDEO on track n #{i}"
#                @width = track.mt_union.video.i_width
#                @height = track.mt_union.video.i_height
            end
            if track.i_type == Vlc::LIBVLC_TRACK_AUDIO
                puts "      AUDIO on track n #{i}"
            end
            
            puts "set_media ** track " + track.inspect
            puts "      i_codec : #{track.i_codec.to_s}" # .unpack('L')
            puts "      i_original_fourcc #{track.i_original_fourcc.to_s}" # .unpack('L')
            puts "      i_id : #{track.i_id.to_s}" # .unpack('i')
            puts "      i_type.to_s : #{track.i_type.to_s}"
            puts "      i_type : #{track.i_type.to_s.unpack('i')}"
                
#            puts "      i_bitrate : #{track.i_bitrate.to_s.unpack('I')}"
#            unless track.psz_language.null?
#                puts "      psz_language : #{track.psz_language.unfold('c')}"
#            end
#            unless track.psz_description.null? 
#                puts "      psz_description : #{track.psz_description.inspect}" if i == 0
#            end
            
            # get a pointer pointing at the beginning of the ruby array 'kind' : Fiddle::Pointer[track.kind[0]]
            # get the address : to_i
            # create a pointer of length 8 bytes at that address, should be a pointer to the element of the 'fake' union
            fp = Fiddle::Pointer.new(Fiddle::Pointer[track.kind[0]].to_i, 8)
            puts "      MT_union Video i_height? : #{Vlc::Video_track.new(fp.ref).i_height.to_s.unpack("I")}" if i == 0
            puts "      MT_union Video i_frame_rate_den? : #{Vlc::Video_track.new(fp.ref).i_frame_rate_den.to_s.unpack("I")}" if i == 0
            puts "      MT_union Audio i_channels? : #{Vlc::Audio_track.new(fp.ref).i_channels.to_s.unpack("I")}" if i == 1

        end
        Vlc::libvlc_media_tracks_release(tracks_alloc, n_tracks);
        
        true
    end
    
    def path=(path)
        stop if playing?
        @path = path
        @loaded = load_media(@path)
        # self_t->init = 0;
        play if @loaded && @autoplay
    end
    
    def play
        libvlc_media_player_play(@player)
    end
    
    def pause
        libvlc_media_player_pause(@player)
    end
    
    def stop
        libvlc_media_player_stop(@player)
    end
    
    def playing?
#        if (self_t->init == 1) # TODO check if needed on medialistplayer
        libvlc_media_player_is_playing(@player) == 1 ? true : false
    end
    
    def volume
        # software volume in percents (0 = mute, 100 = nominal / 0dB) 
        libvlc_audio_get_volume(@player)
    end
    
    def volume=(vol)
        # software volume in percents (0 = mute, 100 = nominal / 0dB) 
        libvlc_audio_set_volume(@player, vol)
    end
    
    def length
        # in ms, or -1 if there is no media.
        libvlc_media_player_get_length(@player) #(bug, might be removed)
    end
    
    def time
        # in ms
        libvlc_media_player_get_time(@player)
    end
    
    def time=(time)
        # in ms
        libvlc_media_player_set_time(@player, time)
    end
    
    def position
        #  percentage between 0.0 and 1.0
        libvlc_media_player_get_position(@player)
    end
     
    def position=(pos)
        #  percentage between 0.0 and 1.0
        libvlc_media_player_get_position(@player, pos)
    end
    
    # redirecting to C video methods
    def show; @video.show; end
    def hide; @video.hide; end
    def displace(x, y); @video.displace(x, y); end
    
end


def video(path, attr=nil)
    # self is the calling app
    Shoes::VideoVlc.new(self, path, attr)
end

def audio(path, attr=nil)
    Shoes::VideoVlc.new(self, path, attr, true)
end
