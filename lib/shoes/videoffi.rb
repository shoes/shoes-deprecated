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
  
  Media_track = struct [
    'unsigned int i_codec',
    'unsigned int i_original_fourcc',
    'int i_id',
    'int i_type', # libvlc_track_type_t
    
    # Codec specific
    'int i_profile',
    'int i_level',
    
    # a void pointer representing an union
    'void* kind',
    
    'unsigned int i_bitrate',
    'char *psz_language',
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
    extern 'void libvlc_release( libvlc_instance_t* )'
    extern 'const char* libvlc_errmsg(void)'
  
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
    extern 'void libvlc_media_player_release( libvlc_media_player_t* )'
#    extern 'void libvlc_media_player_set_media( libvlc_media_player_t*, libvlc_media_t* )'
#    extern 'int libvlc_media_player_play( libvlc_media_player_t* )'
#    extern 'void libvlc_media_player_stop( libvlc_media_player_t* )'
#    extern 'void libvlc_media_player_pause( libvlc_media_player_t* )'
#    extern 'int libvlc_media_player_is_playing( libvlc_media_player_t*)'
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
    extern 'void libvlc_media_list_release( libvlc_media_list_t* )'
    extern 'int libvlc_media_list_add_media( libvlc_media_list_t*, libvlc_media_t* )'
    extern 'int libvlc_media_list_remove_index( libvlc_media_list_t*, int )'
    extern 'int libvlc_media_list_count( libvlc_media_list_t* )'
    extern 'void libvlc_media_list_lock( libvlc_media_list_t* )'
    extern 'void libvlc_media_list_unlock( libvlc_media_list_t* )'
  
    ### Media List Player
    extern 'libvlc_media_list_player_t* libvlc_media_list_player_new(libvlc_instance_t* )'
    extern 'void libvlc_media_list_player_release( libvlc_media_list_player_t* )'
#     Not avalaible in vlc 2.2.0
#     extern 'libvlc_media_player_t* libvlc_media_list_player_get_media_player( libvlc_media_list_player_t* )'
    extern 'void libvlc_media_list_player_set_media_player( libvlc_media_list_player_t*, libvlc_media_player_t* )'
    extern 'void libvlc_media_list_player_set_media_list( libvlc_media_list_player_t*, libvlc_media_list_t* )'
    extern 'int libvlc_media_list_player_play( libvlc_media_list_player_t* )'
    extern 'void libvlc_media_list_player_stop( libvlc_media_list_player_t* )'
    extern 'void libvlc_media_list_player_pause( libvlc_media_list_player_t* )'
    extern 'int libvlc_media_list_player_is_playing( libvlc_media_list_player_t* )'
    extern 'int libvlc_media_list_player_next( libvlc_media_list_player_t* )'
    extern 'int libvlc_media_list_player_previous( libvlc_media_list_player_t* )'
    extern 'int libvlc_media_list_player_play_item_at_index( libvlc_media_list_player_t*, int )'
    extern 'int libvlc_media_list_player_play_item( libvlc_media_list_player_t*, libvlc_media_t* )'
  
    ### Audio controls
    extern 'int libvlc_audio_get_volume( libvlc_media_player_t* )'
    extern 'int libvlc_audio_set_volume( libvlc_media_player_t*, int )'
  
    ### Video Controls
    extern 'int libvlc_video_get_size( libvlc_media_player_t*, unsigned int, unsigned int*, unsigned int* )'
  
    @@vlc_import_done = true
  end
  
  # Load native library.
  def self.load_lib
    if ENV['VLC_APP_PATH']
      vlcpath = ENV['VLC_APP_PATH']
      Dir.chdir(File.dirname(vlcpath)) do 
        #puts "VLC opening with ENV: #{vlcpath}"
        begin
          dlload(File.basename(vlcpath))
        rescue
          upraise "Sorry, #{vlcpath} doesn't load - is it correct"
        end
      end
    else
      upraise "I can't find 'VLC_APP_PATH' environment variable.\n" \
              "Please check with the Cobbler ('Maintain Shoes' at startup splash)"
    end
  
    # do a version check to make sure it is 2.1 or 2.2
    extern 'const char* libvlc_get_version()'
    versionstr = libvlc_get_version().to_s
    version = versionstr[/\d.\d/]
    verno = version.to_f
    if verno < 2.1
      upraise "You need a newer VLC: 2.1 or better"
    end
    info "using VLC: #{versionstr}"
    import_symbols() unless @@vlc_import_done
    
    # import some platform specific symbols
    unless RUBY_PLATFORM =~ /darwin/
      extern 'void libvlc_video_set_mouse_input( libvlc_media_player_t *, unsigned int )'
      extern 'void libvlc_video_set_key_input( libvlc_media_player_t *, unsigned int )'
    end
  
    # just in case ... other functions in Fiddle::Importer are using it
    # The variable is declared in 'dlload'
    @func_map = VLC_FUNCTIONS_MAP
  end
  
end

def upraise(message)
  Shoes.show_log
  raise message
end

class Shoes::VideoVlc
  include Vlc

  attr_accessor :autoplay
  attr_reader :path, :player, :loaded, :version, :have_video_track, :have_audio_track, 
              :video_track_width, :video_track_height

  def initialize(app, path, attr=nil)
    @path = path
    attr ||= {}
    @autoplay = attr[:autoplay] || false
    
    Vlc.load_lib
    @version = libvlc_get_version
    
    # user gives an array of string options i.e.
    #   vlc_options: ["--no-xlib", "--no-video-title-show"]
    #   libvlc expects : libvlc_new(2, ["--no-xlib", "--no-video-title-show"].pack('p2'))
    opts = attr.delete(:vlc_options)
    sz, args = 0, nil
    if opts
        sz = opts.size
        args = opts.pack("p#{sz}")
    end
    
    @vlci = libvlc_new(sz, args)
    if @vlci.null?
        upraise "Unable to initialize libvlc\nlibvlc_new() failed with  :vlc_options => #{opts.inspect}\n" \
               "and returned : \n#{@vlci.inspect}" 
    end
    
    @player = libvlc_media_player_new(@vlci)
    @list_player = libvlc_media_list_player_new(@vlci)
    libvlc_media_list_player_set_media_player(@list_player, @player)
    @medialist = libvlc_media_list_new(@vlci)
    libvlc_media_list_player_set_media_list(@list_player, @medialist)
    
    @loaded = load_media @path
    vol = attr[:volume] || 85
    libvlc_audio_set_volume(@player, vol)
    attr[:video_width] = video_track_width if video_track_width
    attr[:video_height] = video_track_height if video_track_height
    
    @video = app.video_c attr

    # we must wait for parent (hence video itself) to be drawn in order to get the widget drawable
    # (keep "start" event free for possible use in Shoes script)
    # using "animate" method because of the underlying "g_timeout_add" function (let's have peaceful relations with gtk)
    @wait_ready = app.animate(100) do |fr|
      if @video.drawable_ready?
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
        
        # By default, mouse and keyboard events are handled by 
        # the LibVLC video widget, disabling them now!
        # only avalaible on x11 and win32
        unless RUBY_PLATFORM =~ /darwin/
          libvlc_video_set_mouse_input(@player, 0)
          libvlc_video_set_key_input(@player, 0)
        end

        play if @loaded && @autoplay
        
        @wait_ready.remove
        @wait_ready = nil
      end
    end

  end

  def load_media(path)
    @have_video_track = @have_audio_track = nil
    @video_track_width = @video_track_height = nil
    
    # empty string given, nothing to do, waiting for media to be loaded later
    return nil if @path.empty?
    
    @media = 
    if path =~ %r{://}
      libvlc_media_new_location(@vlci, path)
    else
      upraise "Sorry, That File doesn't exists :\n#{path} !" unless File.exists? path
      libvlc_media_new_path(@vlci, path)
    end
    
    if @media.null?  # TODO media is not a null pointer in case of failure ...
        upraise "Sorry, i can't load that media! :\n#{path}"
    end

    access_media_list do |medialist|
      libvlc_media_list_remove_index(medialist, 0) if (libvlc_media_list_count(medialist) != 0)
      libvlc_media_list_add_media(medialist, @media);
    end

    ### Get some info about the tracks inside the @media 
    ### how to deal with a C array OUT parameter (tracks_buf.ref)
    libvlc_media_parse(@media)
    
    tracks_buf = Fiddle::Pointer.malloc(Media_track.size)
    n_tracks = libvlc_media_tracks_get( @media, tracks_buf.ref )
    libvlc_media_release(@media)

    ptr_size, unpkf = 8, 'Q'      # 64 bits arch
    if Fiddle::SIZEOF_VOIDP == 4  # 32 bits arch
      ptr_size = 4
      unpkf = 'L'
    end
#    We could also have taken the unpack route
#     (0...n_tracks).each do |i|
#        tr_p = tracks_buf[ptr_size*i, ptr_size]
#        tr_pv = Fiddle::Pointer.new(tr_p.unpack(unpkf)[0])[0, Media_track.size] 
#        i_type = tr_pv[12,4].unpack('i')[0]
#        ...
#     end

    (0...n_tracks).each do |i|
      tr_p = tracks_buf[ptr_size*i, ptr_size]   # get the pointer (4/8 bytes) at index i of the tracks_buf array
      tr_pa = tr_p.unpack(unpkf)[0]             # get the address (integer) of that pointer
      mdt = Media_track.new(tr_pa)              # use Fiddle structure wrapper

      if mdt.i_type == LIBVLC_TRACK_VIDEO
        @have_video_track = true
        v = Video_track.new Fiddle::Pointer[mdt.kind]
        @video_track_width = v.i_width
        @video_track_height = v.i_height
      end

      if mdt.i_type == LIBVLC_TRACK_AUDIO
        @have_audio_track = true
#        a = Audio_track.new Fiddle::Pointer[mdt.kind]
      end
#       puts "mdt.psz_language : #{mdt.psz_language}" unless mdt.psz_language.null?
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
    
    if @loaded
      if @video.style[:using_video_dim] # no dimensions provided
        if @have_video_track    # no dimensions provided, relying on video track size
          @video.show
          @video.style(width: video_track_width, height: video_track_height)
        else                    # no dimensions provided, nothing to show (audio track)
          @video.style(width: 0, height: 0)
          @video.hide
        end
      end
      play if @autoplay
    end
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

  def play_media(path)
      media = libvlc_media_new_path(@vlci, path)
      libvlc_media_list_player_play_item(@list_player, media)
  end
  
  def play_list_add(path)
      r = nil
      access_media_list do |medialist|
        r = libvlc_media_list_add_media(medialist, libvlc_media_new_path(@vlci, path))
      end
      r == 0 ? true : false
  end
  
  def play_at(index)
      r = libvlc_media_list_player_play_item_at_index(@list_player, index)
      return r == 0 ? true : false
  end

  # redirecting to shoes C video methods
  def show; @video.show; end
  def hide; @video.hide; end
  def toggle; @video.toggle; end
  def parent; @video.parent; end
  def remove
    libvlc_media_list_release(@medialist)
    libvlc_media_player_release(@player)
    libvlc_media_list_player_release(@list_player)
    libvlc_release(@vlci)
    @vlci  = @list_player = @player = @medialist = nil
    @video.remove
    @video = nil
  end
  
  def style(attrb=nil)
    return @video.style if attrb == nil
    @video.style(attrb)
  end
  def displace(x, y); @video.displace(x, y); end
  def move(x, y); @video.move(x, y); end
  def width; @video.width; end    # widget width, not video track width
  def height; @video.height; end  # ditto
  def left; @video.left; end
  def top; @video.top; end

end


def video(path, attr=nil)
  if path.nil? || (not path.kind_of? String)
    upraise "You must, at least, provide an empty path as first agument ! :\n "\
          "video(\"\", optional_attriutes) or audio(\"\", optional_attriutes)\n"
  end
  # self is the calling app
  Shoes::VideoVlc.new(self, path, attr)
end

# convenience method, hiding away the widget
def audio(path, attr=nil)
  attr ||= {}
  attr.merge!( {width: 0, height: 0, hidden: true} )
  video(path, attr)
end
