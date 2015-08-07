# Build a 64 bit Linux Tight Shoes (from a 64 bit host)
# In this case Unbuntu 14.04 to debian 7.2 in a chroot.
# You should modify your custom.yaml
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
  APP['GTK'] = custmz['Gtk'] if custmz['Gtk']
else
  # define where your deps are
  ShoesDeps = ""
  EXT_RUBY = "/usr/local"
  APP['GTK'] = "gtk+-2.0"
end
#ENV['DEBUG'] = "true" # turns on the tracing log
#ENV['GDB'] = "" # compile -g,  strip symbols when not defined
CHROOT = ShoesDeps
SHOES_TGT_ARCH = 'x86_64-linux'
SHOES_GEM_ARCH = "#{Gem::Platform.local}"
# Specify where the Target system binaries live. 
# Trailing slash is important.
TGT_SYS_DIR = "#{CHROOT}/"
# Setup some shortcuts for the library locations
arch = 'x86_64-linux-gnu'
uldir = "#{TGT_SYS_DIR}usr/lib"
ularch = "#{TGT_SYS_DIR}usr/lib/#{arch}"
larch = "#{TGT_SYS_DIR}lib/#{arch}"
# Set appropriately
CC = "gcc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.1.pc"
pkggtk ="#{ularch}/pkgconfig/#{APP['GTK']}.pc" 
# Use Ruby or curl for downloads
RUBY_HTTP = true
file_list = ["shoes/console/*.c"] + %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Target environment
#CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
#PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end
LINUX_CFLAGS << " -DRUBY_HTTP" if RUBY_HTTP
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -Wno-unused-but-set-variable"
LINUX_CFLAGS << " -DGTK3" unless APP['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include "
LINUX_CFLAGS << `pkg-config --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkg-config --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/ " 

LINUX_LIB_NAMES = %W[ungif jpeg]

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L#{ularch} -lrt -ldl -lcrypt -lm "

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CURL_LDFLAGS if !RUBY_HTTP} #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} "

SOLOCS = {}
SOLOCS['curl'] = "#{curlloc}/libcurl.so.4" if !RUBY_HTTP
SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{uldir}/libgif.so.4" # because Suse wants it
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.8"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2"
SOLOCS['pcre'] = "#{larch}/libpcre.so.3"
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.0"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
