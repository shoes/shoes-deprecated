#
# Build a 32 bit Linux Shoes (from a 64 bit host)
#
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  ENV['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  #APP['GTK'] = custmz['Gtk'] if custmz['Gtk']
else
  abort "You need a custom.yaml"
end
APP['GTK'] = "gtk+-3.0"
#ENV['DEBUG'] = "true" # turns on the tracing log
CHROOT = ShoesDeps
SHOES_TGT_ARCH = 'i686-linux'
SHOES_GEM_ARCH = "#{Gem::Platform.local}"
# Specify where the Target system binaries live. 
# Trailing slash is important.
TGT_SYS_DIR = "#{CHROOT}/"
# Setup some shortcuts for the library locations
arch = 'i386-linux-gnu'
uldir = "#{TGT_SYS_DIR}usr/lib"
ularch = "#{TGT_SYS_DIR}usr/lib/#{arch}"
larch = "#{TGT_SYS_DIR}lib/#{arch}"
# Set appropriately
CC = "gcc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.2.pc"
pkggtk ="#{ularch}/pkgconfig/gtk+-3.0.pc" 
gtk_extra_list = Dir["shoes/native/*.c"] - ["shoes/native/gtk.c"]
console_list  = Dir["shoes/console/*.c"]
file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + gtk_extra_list + ["shoes/*.c"] +
     ["shoes/plot/*.c"] + ["shoes/types/*.c"] + ["shoes/native/gtk/*.c"] + console_list
#file_list = ["shoes/console/*.c"] + ["shoes/native/*.c"] + ["shoes/http/rbload.c"] + ["shoes/*.c"] +
#  ["shoes/plot/*.c"]
#file_list << "shoes/video/video.c" 
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Target environment
CAIRO_LIB = `pkg-config --libs cairo`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end

LINUX_CFLAGS << " -DSHOES_GTK -Wno-unused-but-set-variable -Wno-unused-variable"
LINUX_CFLAGS << " -DRUBY_HTTP" 
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include "
LINUX_CFLAGS << `pkg-config --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkg-config --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/ " 
LINUX_CFLAGS << "-I/usr/include/librsvg-2.0/librsvg "
MISC_LIB = ' /usr/lib/i386-linux-gnu/librsvg-2.so'

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# 
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L#{ularch} -lrt -ldl -lcrypt -lm "

LINUX_LIBS =  %W[ungif jpeg].map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

SOLOCS = {}
SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{uldir}/libgif.so.4"
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.8"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2"
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.0"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
SOLOCS['ffi'] = "#{ularch}/libffi.so" 
SOLOCS['rsvg2'] = "#{ularch}/librsvg-2.so"
