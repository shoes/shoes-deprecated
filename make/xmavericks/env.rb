# xmavericks (10.9) build Assumes:
# (1) deps are in BREWLOC
# (2) Ruby was built -C --enable-shared --enable-load-relative & installed in BREWLOC
# (3) 10.9 SDK is installed (ln -s if need) in Xcode.app/....
include FileUtils

cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
if File.exists? cf
  custmz = YAML.load_file(cf)
  BREWLOC = custmz['Deps']
  EXT_RUBY = custmz['Ruby'] ? custmz['Ruby'] : RbConfig::CONFIG['prefix']
  puts "For #{EXT_RUBY}"
  ENV['GDB'] = 'basic' if custmz['Debug'] == true
  ENV['CDEFS'] = custmz['CFLAGS'] if custmz['CFLAGS']
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  ENV['CDEFS'] = custmz['CFLAGS'] if custmz['CFLAGS']
  ENV['SQLLOC'] = BREWLOC
else
  abort "You must have a #{TGT_ARCH}-custom.yaml"
end

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list =  %w{shoes/console/tesi.c shoes/console/cocoa-term.m shoes/native/cocoa.m shoes/http/nsurl.m} + ["shoes/*.c"]
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end
if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O"
end

ADD_DLL = []

# nothing is going to change for 10.9 deps - don't bother with pkg-config
# because it does go wrong in this situation.
GLIB_CFLAGS   = "-I#{BREWLOC}/include/glib-2.0 -I#{BREWLOC}/lib/glib-2.0/include"
GLIB_CFLAGS << " -I#{BREWLOC}/include/librsvg-2.0/librsvg -I#{BREWLOC}/include/gdk-pixbuf-2.0/"
GLIB_LDFLAGS  = "-L#{BREWLOC}/lib -lglib-2.0 -lgobject-2.0 -lintl #{BREWLOC}/lib/librsvg-2.2.dylib"
CAIRO_CFLAGS  = "-I#{BREWLOC}/include/cairo"
CAIRO_LDFLAGS = "-L#{BREWLOC}/lib -lcairo"
PANGO_CFLAGS  = "-I#{BREWLOC}/include/pango-1.0"
PANGO_LDFLAGS = "-L#{BREWLOC}/lib -lpango-1.0"
RUBY_CFLAGS   = "-I#{BREWLOC}/include/ruby-2.1.0/x86_64-darwin13.0 -I#{BREWLOC}/include/ruby-2.1.0 "
RUBY_LDFLAGS  = "-L#{BREWLOC}lib/ -Wl,-undefined,dynamic_lookup -Wl,-multiply_defined,suppress -lruby.2.1.0 -lpthread -ldl -lobjc "

LINUX_CFLAGS << " -I#{BREWLOC}/include #{GLIB_CFLAGS} #{RUBY_CFLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"

LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]

LINUX_CFLAGS << " -DRUBY_1_9 "

DLEXT = "dylib"
#LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{RbConfig::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -fpascal-strings -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'


OSX_SDK = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk'
ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.9'
#LINUX_CFLAGS << ' -mmacosx-version-min=10.9'
#LINUX_LDFLAGS << ' -mmacosx-version-min=10.9'
LINUX_CFLAGS << ' -Wno-incompatible-pointer-types-discards-qualifiers'

OSX_ARCH = '-arch x86_64'
# These env vars are used in ftsearch, chipmunk extconf.rb
SHOES_TGT_ARCH = SHOES_GEM_ARCH ='x86_64-darwin13.0'
ENV['CC'] = CC
ENV['TGT_RUBY_PATH'] = EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '2.1.0'  # library version - all 2.1.x rubys
ENV['SYSROOT'] = " -isysroot #{OSX_SDK} #{OSX_ARCH}"

LINUX_CFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH}"
LINUX_LDFLAGS << " -isysroot #{OSX_SDK} #{OSX_ARCH} -L#{BREWLOC}/lib/ #{GLIB_LDFLAGS}"

LINUX_LIBS = " -l#{RUBY_SO} -L#{BREWLOC}/lib -l cairo -L#{BREWLOC}/lib -lpangocairo-1.0 -L#{BREWLOC}/lib -lgif -ljpeg"
LINUX_LIBS << " -L#{TGT_DIR} #{CAIRO_LDFLAGS} #{PANGO_LDFLAGS} #{GLIB_LDFLAGS}"
