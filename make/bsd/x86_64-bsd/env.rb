# Build a 64 bit Linux Tight Shoes (from a 64 bit host)
# In this case Unbuntu 14.04 to debian 7.2 in a chroot.
# You should modify your custom.yaml
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  APP['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
else
  abort "missing custom.yaml"
end

APP['GTK'] = 'gtk+-3.0' # installer needs this to name the output
SHOES_TGT_ARCH = 'x86_64-linux'
SHOES_GEM_ARCH = "#{Gem::Platform.local}"
# Setup some shortcuts for the library locations
arch = 'x86_64-linux-gnu'
uldir = "#{ShoesDeps}/usr/lib"
ularch = "#{ShoesDeps}/usr/lib/#{arch}"
larch = "#{ShoesDeps}/lib/#{arch}"
lcllib = "/usr/local/lib"
# Set appropriately
CC = "cc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.3.pc"
pkggtk ="/usr/local/libdata/pkgconfig/gtk+-3.0.pc" 
# Use Ruby or curl for downloads
RUBY_HTTP = true

ADD_DLL = []

# Target environment
#CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkgconf --libs cairo`.strip
#PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkgconf --libs pango`.strip

png_lib = 'png'

if APP['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end
LINUX_CFLAGS << " -DRUBY_HTTP -DBSD" 
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -Wno-unused-variable"
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include "
LINUX_CFLAGS << `pkgconf --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkgconf --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include/ " 
#LINUX_CFLAGS << "-I/usr/include/librsvg-2.0/librsvg "
#MISC_LIB = ' /usr/lib/x86_64-linux-gnu/librsvg-2.so'

LINUX_CFLAGS <<  " -I/usr/local/include/librsvg-2.0/librsvg "
MISC_LIB =  " /usr/local/lib/librsvg-2.so"
#LINUX_LIB_NAMES = %W[ungif jpeg]
LINUX_LIB_NAMES = %W[jpeg]

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkgconf --libs "#{pkggtk}"`.strip+" "
# use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-lelf -lexecinfo -lprocstat -lthr -lcrypt -lm  "

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CURL_LDFLAGS if !RUBY_HTTP} #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

SOLOCS = {}
#SOLOCS['ungif'] = "#{uldir}/libungif.so.4.1.6"
SOLOCS['gif'] = "/usr/local/lib/libgif.so.7.0.0" 
SOLOCS['jpeg'] = "/usr/local/lib//libjpeg.so.8.1.2"
SOLOCS['libyaml'] = "/usr/local/lib/libyaml-0.so.2.0.4"
SOLOCS['pcre'] = "/usr/local/lib/libpcre.so.1.2.8"
#SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"  #needed ?
SOLOCS['ssl'] = "/usr/lib/libssl.so.8"
SOLOCS['sqlite'] = "/usr/local/lib/libsqlite3.so.0.8.6"
SOLOCS['ffi'] = "/usr/local/lib/libffi.so.6.0.4"
SOLOCS['rsvg2'] = "/usr/local/lib/librsvg-2.so.2.40.17"
SOLOCS['curl'] = "/usr/local/lib/libcurl.so.4.4.0"

# sigh, curl and tyhpoeus - processed in setup.rb
SYMLNK = {}
SYMLNK['libcurl.so.4.4.0'] = ['libcurl.so', 'libcurl.so.4']

