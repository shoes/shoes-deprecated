# Assumes:
# (1) You like suffering. You really should not do this. 
# (1) Ruby is installed with RVM and installed -C --enable-load-relative
# (2) Homebrew installed packages including gtk2 and you've got PKG_CONFIG_PATH
#     setup and you know how to undo things in Homebrew and Xquartz if needed
# (3) You like suffering
include FileUtils
EXT_RUBY = RbConfig::CONFIG['prefix']

# use the platform Ruby claims

ENV['GTK'] = "gtk+-2.0"
CC = ENV['CC'] ? ENV['CC'] : "gcc"
#file_list = ["shoes/*.c"] + %w{shoes/native/cocoa.m shoes/http/nsurl.m}
file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

puts "PKG #{ENV['PKG_CONFIG_PATH']}"
GTK_FLAGS = `pkg-config --cflags gtk+-2.0`.strip
GTK_LIBS = `pkg-config --libs gtk+-2.0`.strip
png_lib = 'png'
LINUX_CFLAGS = " -DRUBY_HTTP -DSHOES_GTK -DSHOES_GTK_OSX "
LINUX_CFLAGS << %[-Wall #{ENV['GLIB_CFLAGS']} -I/usr/local/include #{GTK_FLAGS} -I#{RbConfig::CONFIG['archdir']}]
if RbConfig::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{RbConfig::CONFIG['rubyhdrdir']} -I#{RbConfig::CONFIG['rubyhdrdir']}/#{SHOES_RUBY_ARCH}"
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
LINUX_CFLAGS << " -DRUBY_1_9"

DLEXT = "dylib"
#LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -Wall -fpascal-strings -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

#OSX_SDK = '/Developer/SDKs/MacOSX10.6.sdk'
#ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
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
LINUX_LDFLAGS << " #{OSX_ARCH}"
 
LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

#LINUX_LIBS << " -L#{RbConfig::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB}"
#LINUX_LIBS << " -L#{TGT_DIR}/lib/ruby/#{RUBY_V} #{CAIRO_LIB} #{PANGO_LIB}"
LINUX_LIBS << " -L#{TGT_DIR} #{GTK_LIBS}" #{CAIRO_LIB} #{PANGO_LIB}

