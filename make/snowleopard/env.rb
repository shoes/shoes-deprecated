# Assumes:
# (1) Ruby is installed with RVM and installed -C --enable-load-relative
# (2) {TGT_DIR}/libruby.dylib was copied from rvm before linking.
# (3) {TGT_DIR}/lib/ruby/2.0.0/include was copied before compiling.
# (4) You've compiled zlib 1.2.8 or better
include FileUtils
EXT_RUBY = RbConfig::CONFIG['prefix']
# Where is homebrew - often /usr/local
BREWLOC = '/usr/local'
ZLIBLOC = "/usr/local/opt/zlib/lib"

ENV['SHOES_DEPS_PATH'] = '/usr/local'

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list =  %w{shoes/native/cocoa.m shoes/http/nsurl.m} + ["shoes/*.c"]

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

CAIRO_CFLAGS = "-I#{BREWLOC}/opt/cairo/include/cairo"
CAIRO_LIB = "-L/#{BREWLOC}/opt/cairo/lib"
PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

LINUX_CFLAGS = %[-g -Wall -I#{BREWLOC}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{RbConfig::CONFIG['archdir']}]
if RbConfig::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{RbConfig::CONFIG['rubyhdrdir']} -I#{RbConfig::CONFIG['rubyhdrdir']}/#{SHOES_RUBY_ARCH}"
end
  
LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]

if ENV['GDB']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9"

DLEXT = "dylib"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

OSX_SDK = '/Developer/SDKs/MacOSX10.6.sdk'
ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
LINUX_CFLAGS << ' -DOLD_OSX '

case ENV['SHOES_OSX_ARCH']
when "universal"
  OSX_ARCH = "-arch i386 -arch x86_64"
when "i386"
  OSX_ARCH = '-arch i386'
else
  OSX_ARCH = '-arch x86_64'
end

LINUX_CFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH}"
LINUX_LDFLAGS << " #{OSX_ARCH} -L/usr/local/lib/ "
 
LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " -L#{TGT_DIR} #{CAIRO_LIB} #{PANGO_LIB}"


# Additional Libraries
LINUX_CFLAGS << " -I/usr/local/opt/pixman/include " 
LINUX_LDFLAGS << " -L/usr/local/opt/pixman/lib " 
LINUX_LIBS << " -L/usr/local/opt/pixman/lib "
