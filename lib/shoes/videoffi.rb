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
    
    Media_stats = struct [
        #/* Input */
        'int         i_read_bytes',
        'float       f_input_bitrate',
        #/* Demux */
        'int         i_demux_read_bytes',
        'float       f_demux_bitrate',
        'int         i_demux_corrupted',
        'int         i_demux_discontinuity',
        #/* Decoders */
        'int         i_decoded_video',
        'int         i_decoded_audio',
        #/* Video Output */
        'int         i_displayed_pictures',
        'int         i_lost_pictures',
        #/* Audio output */
        'int         i_played_abuffers',
        'int         i_lost_abuffers',
        #/* Stream output */
        'int         i_sent_packets',
        'int         i_sent_bytes',
        'float       f_send_bitrate'
    ]
    
    @@vlc_import_done = false
    def self.import_symbols    
        typealias "uint32_t", "unsigned int"
        typealias "libvlc_time_t", "long long"
        typealias "libvlc_track_type_t", "int"

        extern 'libvlc_instance_t * libvlc_new( int , const char* )'
        extern 'const char* libvlc_get_version()'
        
        ### Media
        extern 'libvlc_media_t* libvlc_media_new_path(libvlc_instance_t*, const char* )'
        extern 'libvlc_media_t* libvlc_media_new_location(libvlc_instance_t*, const char* )'
        extern 'void libvlc_media_parse( libvlc_media_t* )'
        extern 'int libvlc_media_is_parsed( libvlc_media_t* )'
        extern 'unsigned int libvlc_media_tracks_get( libvlc_media_t*, libvlc_media_track_t*** )'
        extern 'void libvlc_media_tracks_release( libvlc_media_track_t**, unsigned int )'
        extern 'void libvlc_media_release( libvlc_media_t* )'
        extern 'int libvlc_media_get_stats( libvlc_media_t*, struct libvlc_media_stats_t* )'

        ### Media player
        extern 'libvlc_media_player_t* libvlc_media_player_new( libvlc_instance_t* )'
#        extern 'void libvlc_media_player_set_media( libvlc_media_player_t*, libvlc_media_t* )'
#        extern 'int libvlc_media_player_play( libvlc_media_player_t* )'
#        extern 'void libvlc_media_player_stop( libvlc_media_player_t* )'
#        extern 'void libvlc_media_player_pause( libvlc_media_player_t* )'
#        extern 'int libvlc_media_player_is_playing( libvlc_media_player_t*)'
        extern 'uint32_t libvlc_media_player_get_xwindow( libvlc_media_player_t*)'
        extern 'void libvlc_media_player_set_xwindow( libvlc_media_player_t*, uint32_t)'
        extern 'libvlc_time_t libvlc_media_player_get_time( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_time(libvlc_media_player_t*, libvlc_time_t )'
        extern 'libvlc_time_t libvlc_media_player_get_length( libvlc_media_player_t* )'
        extern 'float libvlc_media_player_get_position( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_position( libvlc_media_player_t*, float )'
        
        ### Media List
        extern 'libvlc_media_list_t* libvlc_media_list_new( libvlc_instance_t* )'
        extern 'int libvlc_media_list_add_media( libvlc_media_list_t*, libvlc_media_t* )'
        extern 'int libvlc_media_list_remove_index( libvlc_media_list_t*, int )'
        extern 'int libvlc_media_list_count( libvlc_media_list_t* )'
        extern 'void libvlc_media_list_lock( libvlc_media_list_t* )'
        extern 'void libvlc_media_list_unlock( libvlc_media_list_t* )'
        
        ### Media List Player
        extern 'libvlc_media_list_player_t* libvlc_media_list_player_new(libvlc_instance_t* )'
#        Not avalaible in vlc 2.2.0
#        extern 'libvlc_media_player_t* libvlc_media_list_player_get_media_player( libvlc_media_list_player_t* )'
        extern 'void libvlc_media_list_player_set_media_player( libvlc_media_list_player_t*, libvlc_media_player_t* )'
        extern 'void libvlc_media_list_player_set_media_list( libvlc_media_list_player_t*, libvlc_media_list_t* )'
        extern 'int libvlc_media_list_player_play( libvlc_media_list_player_t* )'
        extern 'void libvlc_media_list_player_stop( libvlc_media_list_player_t* )'
        extern 'void libvlc_media_list_player_pause( libvlc_media_list_player_t* )'
        extern 'int libvlc_media_list_player_is_playing( libvlc_media_list_player_t* )'
        
        ### Audio controls
        extern 'int libvlc_audio_get_volume( libvlc_media_player_t* )'
        extern 'int libvlc_audio_set_volume( libvlc_media_player_t*, int )'
        
        ### Video Controls
        extern 'int libvlc_video_get_size( libvlc_media_player_t*, unsigned int, unsigned int*, unsigned int* )'
        
        @@vlc_import_done = true
    end
    
    # Load native library.
    def self.load_lib(path = nil)
        if path
            @vlc_lib = path
        else
            case RUBY_PLATFORM
            when /mingw/     
                lib = 'libvlc.dll'                 
                @vlc_lib = lib
            when /darwin/                          
               lib = 'libvlc.dylib'               
                @vlc_lib = "/Applications/VLC.app/Contents/MacOS/lib/#{lib}"                      
            when /linux/
                Dir.glob('/usr/lib/libvlc.so*') do |p|
                  @vlc_lib = p if ! File.symlink?(p)
                end
                #@vlc_lib = 'libvlc.so.5.4.0'
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
    attr_reader :path, :player, :width, :height, :loaded, :version,
                :track_width, :track_height
    
    def initialize(app, path, attr=nil, audio=false)
        @path = path
        attr ||= {}
        @autoplay = attr[:autoplay] || false
        
        # what should we do with "--no-xlib"/XInitThreads(), seems controversial ...
        # Do we need threaded xlib in shoes/vlc ?
        @vlci = libvlc_new(0, nil)
        @version = libvlc_get_version
        
        @player = libvlc_media_player_new(@vlci)
        @list_player = libvlc_media_list_player_new(@vlci)
        libvlc_media_list_player_set_media_player(@list_player, @player)
        @medialist = libvlc_media_list_new(@vlci)
        libvlc_media_list_player_set_media_list(@list_player, @medialist)
        
        @loaded = load_media @path
        
        if audio
            attr[:width] = attr[:height] = 0
            attr[:hidden] = true
        end    
        @video = app.video_c attr
        
        
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
    
    def load_media(path)
        @media = 
        if path =~ /:\/\//
            libvlc_media_new_location(@vlci, path)
        else
            libvlc_media_new_path(@vlci, path)
        end
        return nil if (@media.null? or @path == "")
        
        libvlc_media_list_lock(@medialist)
        libvlc_media_list_remove_index(@medialist, 0) if (libvlc_media_list_count(@medialist) != 0) 
        libvlc_media_list_add_media(@medialist, @media);
        libvlc_media_list_unlock(@medialist)
        
        libvlc_media_parse(@media)
#        parsed = libvlc_media_is_parsed(@media)
#        puts "parsed ? #{parsed}"
        libvlc_media_release(@media)
        
        tracks_buf = Fiddle::Pointer[' ' * Media_track.size * 5].ptr
        n_tracks = libvlc_media_tracks_get( @media, tracks_buf.ref )
#        puts " n tracks = #{n_tracks}"

        
        (0...n_tracks).each do |i|
            w_ptr = ' ' * 4
            h_ptr = ' ' * 4
            vid = libvlc_video_get_size(@player, i, w_ptr, h_ptr)
            if vid == 0
                @track_width =  w_ptr.unpack("I")[0]
                @track_height = h_ptr.unpack("I")[0]
            end
            
        end
        libvlc_media_tracks_release(tracks_buf, n_tracks);
        
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
        libvlc_media_list_player_play(@list_player)
    end
    
    def pause
        libvlc_media_list_player_pause(@list_player)
    end
    
    def stop
        libvlc_media_list_player_stop(@list_player)
    end
    
    def playing?
#        if (self_t->init == 1) # TODO check if needed on medialistplayer
        libvlc_media_list_player_is_playing(@list_player) == 1 ? true : false
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
        libvlc_media_player_get_length(@player) #(buggy, might be removed)
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
        libvlc_media_player_set_position(@player, pos)
    end
    
    # redirecting to shoes C video methods
    def show; @video.show; end
    def hide; @video.hide; end
    def parent; @video.parent; end
    def remove; @video.remove; end
    def displace(x, y); @video.displace(x, y); end
    
end


def video(path, attr=nil)
    # self is the calling app
    Shoes::VideoVlc.new(self, path, attr)
end

def audio(path, attr=nil)
    Shoes::VideoVlc.new(self, path, attr, true)
end
