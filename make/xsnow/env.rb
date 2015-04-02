# Snow Leopard (10.6) build Assumes:
# (1) Ruby is installed with RVM and installed -C --enable-shared --enable-load-relative
# (2) {TGT_DIR}/libruby.dylib was copied from rvm before linking.
# (3) {TGT_DIR}/lib/ruby/2.0.0/include was copied before compiling.
# (4) Homebrew installed dependencies for 10.6 are in set in BREWLOC below
# (5) 10.6 SDK is installed and has the X11 includes and libs which you set X11LOC to
include FileUtils
# what system am I running on
osxver = `sw_vers -productVersion`
if osxver =~ /10\.6/
  EXT_RUBY = RbConfig::CONFIG['prefix']
  BREWLOC = "/usr/local"
  X11LOC = "/usr/X11"
else
  # build 10.6 on something other than 10.6"
  EXT_RUBY = "/Users/ccoupe/shoesdeps/10.6/ruby-2.1.5"
  BREWLOC = "/Users/ccoupe/shoesdeps/10.6/brew"  
  X11LOC = "/Users/ccoupe/shoesdeps/10.6/X11"
  ENV['SQLLOC'] = BREWLOC
  
end

# use the platform Ruby claims
# require 'rbconfig' not needed

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list =  %w{shoes/native/cocoa.m shoes/http/nsurl.m} + ["shoes/*.c"]

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []
ENV['DYLD_LIBRARY_PATH'] = '/usr/local/Cellar/cairo/1.10.2/lib:/usr/local/Cellar/cairo/1.10.2/include/cairo'
ENV['LD_LIBRARY_PATH'] = '/usr/local/Cellar/cairo/1.10.2/lib:/usr/local/Cellar/cairo/1.10.2/include/cairo'
ENV['CAIRO_CFLAGS'] = '-I/usr/local/Cellar/cairo/1.12.16_1/include/cairo'
ENV['GLIB_CFLAGS'] = '-I/usr/local/Cellar/glib/2.40.0/include/glib-2.0'
#ENV['PKG_CONFIG_PATH'] = '/opt/X11/lib/pkgconfig' # check spelling X11
ENV['SHOES_DEPS_PATH'] = '/usr/local'


def brewsub ln
  ln.gsub('/usr/local',BREWLOC)
end

# Darwin build environment
#pkg_config = `which pkg-config` != ""
pkg_config = "#{BREWLOC}/bin/pkg-config"
pkgs = `pkg-config --list-all`.split("\n").map {|p| p.split.first} unless not pkg_config
if pkg_config and pkgs.include?("cairo") and pkgs.include?("pango")
  CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || brewsub(`pkg-config --cflags cairo`.strip)
  #CAIRO_CFLAGS = brewsub(`pkg-config --cflags cairo`.strip)
  CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : brewsub(`pkg-config --libs cairo`.strip)
  PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || brewsub(`pkg-config --cflags pango`.strip)
  PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : brewsub(`pkg-config --libs pango`.strip)
else
  # Hack for when pkg-config is not yet installed
  CAIRO_CFLAGS, CAIRO_LIB, PANGO_CFLAGS, PANGO_LIB = "", "", "", ""
end
png_lib = 'png'

LINUX_CFLAGS = %[-g -Wall #{brewsub(ENV['GLIB_CFLAGS'])} -I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{RbConfig::CONFIG['archdir']}]
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
LINUX_CFLAGS << " -DRUBY_1_9 "

DLEXT = "dylib"
#LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

#OSX_SDK = '/Developer/SDKs/MacOSX10.6.sdk'
#ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
OSX_SDK = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk'
ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
LINUX_CFLAGS << ' -DOLD_OSX -Wno-incompatible-pointer-types-discards-qualifiers'

case ENV['SHOES_OSX_ARCH']
when "universal"
  OSX_ARCH = "-arch i386 -arch x86_64"
when "i386"
  OSX_ARCH = '-arch i386'
else
  OSX_ARCH = '-arch x86_64'
end

# These env vars are used in  ftsearch, chipmunk, sqlite3 extconf.rb
SHOES_TGT_ARCH = 'x86_64-darwin10.0'
ENV['CC'] = CC
ENV['TGT_RUBY_PATH'] = EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '2.1.0'  # library version - all 2.1.x rubys
ENV['SYSROOT'] = " -isysroot #{OSX_SDK} #{OSX_ARCH}"

LINUX_CFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH}"
LINUX_LDFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH} -L#{BREWLOC}/lib/ "
 
#LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')
LINUX_LIBS = "-L. -l#{RUBY_SO} -L #{BREWLOC}/lib -l cairo -L #{BREWLOC}/lib -lpangocairo-1.0 -L#{BREWLOC}/lib -lgif -ljpeg"
#LINUX_LIBS << " -L#{RbConfig::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB}"
#LINUX_LIBS << " -L#{TGT_DIR}/lib/ruby/#{RUBY_V} #{CAIRO_LIB} #{PANGO_LIB}"
LINUX_LIBS << " -L#{TGT_DIR} #{CAIRO_LIB} #{PANGO_LIB}"

