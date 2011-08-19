EXT_RUBY = File.exists?("deps/ruby") ? "deps/ruby" : Config::CONFIG['prefix']

# use the platform Ruby claims
require 'rbconfig'

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list = ["shoes/*.c"] + %w{shoes/native/cocoa.m shoes/http/nsurl.m}

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Linux build environment
CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || `pkg-config --cflags cairo`.strip
CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : `pkg-config --libs cairo`.strip
PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || `pkg-config --cflags pango`.strip
PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : `pkg-config --libs pango`.strip
png_lib = 'png'

# TODO: COPIED FROM LINUX MAKEFILE! FIX!!
if ENV['VIDEO']
  VLC_CFLAGS = '-I/usr/include/vlc'
  VLC_LIB = '-llibvlc'
else
  VLC_CFLAGS = VLC_LIB = ''
end

LINUX_CFLAGS = %[-Wall -I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{VLC_CFLAGS} -I#{Config::CONFIG['archdir']}]
if Config::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{Config::CONFIG['rubyhdrdir']} -I#{Config::CONFIG['rubyhdrdir']}/#{RUBY_PLATFORM}"
end
  
LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]

FLAGS.each do |flag|
  LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
end

if ENV['DEBUG']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9" if RUBY_1_9

DLEXT = "dylib"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{Config::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module #{Config::CONFIG["LDFLAGS"]} INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

if ENV['UNIVERSAL']
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc"
  LINUX_LDFLAGS << " -arch i386 -arch ppc"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
elsif ENV['PPC']
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
  LINUX_LDFLAGS << " -arch ppc"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
else
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64"
  LINUX_LDFLAGS << " -arch x86_64"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
end

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " -L#{Config::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB} #{VLC_LIB}"

