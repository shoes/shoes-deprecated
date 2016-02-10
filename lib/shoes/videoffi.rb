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

        #extern 'libvlc_instance_t * libvlc_new( int , const char* )'
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
        ## Platform specific
        extern 'uint32_t libvlc_media_player_get_xwindow( libvlc_media_player_t*)'
        extern 'void libvlc_media_player_set_xwindow( libvlc_media_player_t*, uint32_t )'
        extern 'void* libvlc_media_player_get_hwnd( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_hwnd( libvlc_media_player_t*, void* )'
        extern 'void* libvlc_media_player_get_nsobject( libvlc_media_player_t* )'
        extern 'void libvlc_media_player_set_nsobject( libvlc_media_player_t*, void* )'

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
        extern 'int libvlc_media_list_player_next( libvlc_media_list_player_t* )'
        extern 'int libvlc_media_list_player_previous( libvlc_media_list_player_t* )'

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
            begin
              dlload @vlc_lib
            rescue => e #TODO
              raise "Sorry, No Video support !\n unable to find libvlc :  #{@vlc_lib}"
          end
       else
            case RUBY_PLATFORM
            when /mingw/
                # Oddness - dlload on Windows only works this way
                # so every platform has to
                Dir.chdir('C:/Program Files (x86)/VideoLAN/VLC') do
                  p = Dir.glob('libvlc.dll')
                  begin
                    dlload p[0]
                  rescue => e
                    raise "Sorry, No Video support !\n unable to find libvlc : #{Dir.getwd}  #{p[0]}"
                  end
                end
            when /darwin/
                @vlc_lib = "/Applications/VLC.app/Contents/MacOS/lib/libvlc.dylib"
                begin
                  dlload @vlc_lib
                rescue
                  raise "Sorry, No Video support !\n unable to find libvlc :  #{@vlc_lib}"
                end
            when /linux/
                Dir.glob('/usr/lib/libvlc.so*') do |p|
                  @vlc_lib = p if ! File.symlink?(p)
                end
                begin
                  dlload @vlc_lib
                rescue => e
                  raise "Sorry, No Video support !\n unable to find libvlc :  #{@vlc_lib}"
                end
            else
                raise "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
            end
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
    attr_reader :path, :player, :loaded, :version

    def initialize(app, path, attr=nil)
        @path = path
        attr ||= {}
        @autoplay = attr[:autoplay] || false

        @video = app.video_c attr


        if RUBY_PLATFORM =~ /darwin/
          args = ["--play-and-pause",                  # We want every movie to pause instead of stopping at eof
                "--no-color",                          # Don't use color in output (Xcode doesn't show it)
                "--no-video-title-show",               # Don't show the title on overlay when starting to play
                "--verbose=4",                         # Let's not wreck the logs
                "--no-sout-keep",
                "--vout=macosx",                       # Select Mac OS X video output
                "--text-renderer=quartztext",          # our CoreText-based renderer
                "--extraintf=macosx_dialog_provider"]  #Some extra dialog (login, progress) may come up from here
          # convert args tp C style argv (char *[])
          argv = app.video_mkargv args
          argc = args.length
          @vlci = libvlc_new(argc, argv)
        else 
          # what should we do with "--no-xlib"/XInitThreads(), seems controversial ...
          # Do we need threaded xlib in shoes/vlc ?
          @vlci = libvlc_new(0, nil)
        end
        @version = libvlc_get_version
				raise "vlc version#{@version} #{@vlci.inspect}" if @vlci.null?
        @player = libvlc_media_player_new(@vlci)
        @list_player = libvlc_media_list_player_new(@vlci)
        libvlc_media_list_player_set_media_player(@list_player, @player)
        @medialist = libvlc_media_list_new(@vlci)
        libvlc_media_list_player_set_media_list(@list_player, @medialist)

        @loaded = load_media @path
        vol = attr[:volume] || 85
        libvlc_audio_set_volume(@player, vol)

        # we must wait for parent (hence video itself) to be drawn in order to get the widget drawable
        # (keep "start" event free for possible use in Shoes script)
        # using "animate" method because of the underlying "g_timeout_add" function (let's have peaceful relations with gtk)
        @wait_ready = app.animate(100) do |fr|
          if @video.parent.style[:started]
            @wait_ready.stop
            drID = @video.drawable   # xlib window / HWND / NSView  id

            case RUBY_PLATFORM
              when /linux/
                if libvlc_media_player_get_xwindow(@player) == 0
                    libvlc_media_player_set_xwindow(@player, drID)
                end
              when /mingw/
                if libvlc_media_player_get_hwnd(@player).null?
                  libvlc_media_player_set_hwnd(@player, drID)
                end
              when /darwin/
                 if libvlc_media_player_get_nsobject(@player).null?
                   libvlc_media_player_set_nsobject(@player, drID)
                 end
            end

            play if @loaded && @autoplay

          end
        end

    end

    def load_media(path)
        @media =
        if path =~ /:\/\//
            libvlc_media_new_location(@vlci, path)
        else
            libvlc_media_new_path(@vlci, path)
        end
        return nil if (@media.null? or @path == "")

        access_media_list do |medialist|
            libvlc_media_list_remove_index(medialist, 0) if (libvlc_media_list_count(medialist) != 0)
            libvlc_media_list_add_media(medialist, @media);
        end

        libvlc_media_parse(@media)
#        parsed = libvlc_media_is_parsed(@media)
#        puts "parsed ? #{parsed}"

        tracks_buf = Fiddle::Pointer[' ' * Media_track.size * 5].ptr
        n_tracks = libvlc_media_tracks_get( @media, tracks_buf.ref )
#        puts " n tracks = #{n_tracks}"
        libvlc_media_release(@media)

        (0...n_tracks).each do |i|
            # gives the size displayed on screen not the internal "real size"
            # Back to fidddling with tracks_buf !!! >_>
#            w_ptr = ' ' * 4
#            h_ptr = ' ' * 4
#            vid = libvlc_video_get_size(@player, i, w_ptr, h_ptr)
#            puts " vid = #{vid}"
#            if vid == 0 # Doesn't work on 1rst pass
#                @track_width =  w_ptr.unpack("I")[0]
#                @track_height = h_ptr.unpack("I")[0]
#                puts "width : #{@track_width}"
#                puts "height : #{@track_height}"
#            end
        end
        libvlc_media_tracks_release(tracks_buf, n_tracks);

        true
    end

    def access_media_list
        libvlc_media_list_lock(@medialist)
        yield @medialist
        libvlc_media_list_unlock(@medialist)
    end

    def path=(path)
        stop if playing?
        @path = path
        @loaded = load_media(@path)
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

    def next_media
        r = libvlc_media_list_player_next(@list_player)
        return r == 0 ? true : false
    end

    def previous_media
        r = libvlc_media_list_player_previous(@list_player)
        return r == 0 ? true : false
    end

    # redirecting to shoes C video methods
    def show; @video.show; end
    def hide; @video.hide; end
    def toggle; @video.toggle; end
    def parent; @video.parent; end
    def remove; @video.remove; end
    def style(attr=nil); @video.style(attr); end
    def displace(x, y); @video.displace(x, y); end
    def move(x, y); @video.move(x, y); end
    def width; @video.width; end    # widget width, not video track width
    def height; @video.height; end  # ditto
    def left; @video.left; end
    def top; @video.top; end

end


def video(path, attr=nil)
    # self is the calling app
    Shoes::VideoVlc.new(self, path, attr)
end

# convenience method, hiding away the widget
def audio(path, attr=nil)
    attr ||= {}
    attr.merge!( {width: 0, height: 0, hidden: true} )
    Shoes::VideoVlc.new(self, path, attr)
end
