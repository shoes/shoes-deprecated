# Assumes:
# (1) Ruby is installed with RVM and installed -C --enable-load-relative
# (2) {TGT_DIR}/libruby.dylib was copied from rvm before linking.
# (3) {TGT_DIR}/lib/ruby/2.0.0/include was copied before compiling.
# (4) You've compiled zlib 1.2.8 or better
include FileUtils
EXT_RUBY = RbConfig::CONFIG['prefix']
BREWLOC = "/usr/local"
ZLIBLOC = "/usr/local/opt/zlib/lib"
# use the platform Ruby claims
# require 'rbconfig' not needed

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list =  %w{shoes/native/cocoa.m shoes/http/nsurl.m} + ["shoes/*.c"]

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Darwin build environment
=begin
pkg_config = `which pkg-config` != ""
pkgs = `pkg-config --list-all`.split("\n").map {|p| p.split.first} unless not pkg_config
if pkg_config and pkgs.include?("cairo") and pkgs.include?("pango")
  CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || `pkg-config --cflags cairo`.strip
  CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : `pkg-config --libs cairo`.strip
  PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || `pkg-config --cflags pango`.strip
  PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : `pkg-config --libs pango`.strip
else
  # Hack for when pkg-config is not yet installed
  CAIRO_CFLAGS, CAIRO_LIB, PANGO_CFLAGS, PANGO_LIB = "", "", "", ""
=end
CAIRO_CFLAGS = "-I#{BREWLOC}/opt/cairo/include/cairo"
CAIRO_LIB = "-L#{BREWLOC}/opt/cairo/lib"
PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

LINUX_CFLAGS = %[-g -Wall  -I#{BREWLOC}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{RbConfig::CONFIG['archdir']}]
if RbConfig::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{RbConfig::CONFIG['rubyhdrdir']} -I#{RbConfig::CONFIG['rubyhdrdir']}/#{SHOES_RUBY_ARCH}"
end
  
LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]

#FLAGS.each do |flag|
#  LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
#end

if ENV['DEBUG']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9"

DLEXT = "dylib"
#LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -Wno-incompatible-pointer-types-discards-qualifiers"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

#OSX_SDK = '/Developer/SDKs/MacOSX10.6.sdk'
#ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
#LINUX_CFLAGS << ' -DOLD_OSX '
OSX_SDK = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk'
ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.9'

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

#LINUX_LIBS << " -L#{RbConfig::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB}"
#LINUX_LIBS << " -L#{TGT_DIR}/lib/ruby/#{RUBY_V} #{CAIRO_LIB} #{PANGO_LIB}"
LINUX_LIBS << " -L#{TGT_DIR} #{CAIRO_LIB} #{PANGO_LIB}"


# Additional Libraries
LINUX_CFLAGS << " -I/usr/local/opt/pixman/include " #-I/usr/local/Cellar/pixman/0.32.6/include/pixman-1"
LINUX_LDFLAGS << " -L/usr/local/opt/pixman/lib " #-L/usr/local/Cellar/pixman/0.32.6/lib"
LINUX_LIBS << " -L/usr/local/opt/pixman/lib "
