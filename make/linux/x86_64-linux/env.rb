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
CC = "gcc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.3.pc"
pkggtk ="#{ularch}/pkgconfig/gtk+-3.0.pc" 
# Use Ruby or curl for downloads
RUBY_HTTP = true

ADD_DLL = []

# Target environment
#CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
#PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

if APP['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end
LINUX_CFLAGS << " -DRUBY_HTTP" 
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -Wno-unused-but-set-variable -Wno-unused-variable"
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include "
LINUX_CFLAGS << `pkg-config --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkg-config --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include/ " 
LINUX_CFLAGS << "-I/usr/include/librsvg-2.0/librsvg "
MISC_LIB = ' /usr/lib/x86_64-linux-gnu/librsvg-2.so'

LINUX_LIB_NAMES = %W[ungif jpeg]

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L#{ularch} -lrt -ldl -lcrypt -lm "

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CURL_LDFLAGS if !RUBY_HTTP} #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

SOLOCS = {}
SOLOCS['ungif'] = "#{uldir}/libungif.so.4.1.6"
SOLOCS['gif'] = "#{uldir}/libgif.so.4.1.6" # because Suse wants it
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.8.4.0"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2.0.2"
SOLOCS['pcre'] = "#{larch}/libpcre.so.3"
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.0"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
SOLOCS['ffi'] = "#{ularch}/libffi.so.5.0.10"
SOLOCS['rsvg2'] = "#{ularch}/librsvg-2.so.2.36.1"
SOLOCS['curl'] = "#{lcllib}/libcurl.so.4.4.0"

# sigh, curl and tyhpoeus - processed in setup.rb
SYMLNK = {}
SYMLNK['libcurl.so.4.4.0'] = ['libcurl.so', 'libcurl.so.4']

