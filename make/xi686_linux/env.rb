#
# Build a 32 bit Linux Shoes (from a 64 bit host)
# I have debian wheezy 7.1 i386 in an schroot directory. I built ruby
# in there (/usr/local) with the magic of X86_64 multi_arch.
# Currently, you must schroot and then do the rake. Also, in your
# schroot set the PATH to not include rvm.
#ENV['DEBUG'] = "true" # turns on the tracing log
#APP['GTK'] = "gtk+-3.0" # pick this or "gtk+-2.0"
APP['GTK'] = "gtk+-2.0"
# I don't recommend try to copy Gtk2 -it only works mysteriously
COPY_GTK = false 
#ENV['GDB'] = "SureYouBetcha" # compile -g,  strip symbols when undefined
CHROOT = ""
# Where does ruby code live?
EXT_RUBY = "#{CHROOT}/usr/local"
SHOES_TGT_ARCH = 'i686-linux'
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

pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.1.pc"
pkggtk ="#{ularch}/pkgconfig/#{APP['GTK']}.pc" 

file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]

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

LINUX_CFLAGS << " -DSHOES_GTK -Wno-unused-but-set-variable" 
LINUX_CFLAGS << " -DRUBY_HTTP" 
LINUX_CFLAGS << " -DGTK3" unless APP['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include "
LINUX_CFLAGS << `pkg-config --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkg-config --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/ " 

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# 
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L#{ularch} -lrt -ldl -lcrypt -lm "

LINUX_LIBS =  %W[ungif jpeg].map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} "

SOLOCS = {}
SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{uldir}/libgif.so.4"
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.8"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2"
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.0"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
if APP['GTK'] == 'gtk+-2.0' && COPY_GTK == true
  SOLOCS['gtk2'] = "#{ularch}/libgtk-x11-2.0.so.0"
  SOLOCS['gdk2'] = "#{ularch}/libgdk-x11-2.0.so.0"
  SOLOCS['atk'] = "#{ularch}/libatk-1.0.so.0"
  SOLOCS['gio'] = "#{ularch}/libgio-2.0.so.0"
  SOLOCS['pangoft2'] = "#{ularch}/libpangoft2-1.0.so.0"
  SOLOCS['pangocairo'] = "#{ularch}/libpangocairo-1.0.so.0"
  SOLOCS['gdk_pixbuf'] = "#{ularch}/libgdk_pixbuf-2.0.so.0"
  SOLOCS['cairo'] = "#{ularch}/libcairo.so.2"
  SOLOCS['pango'] = "#{ularch}/libpango-1.0.so.0"
  SOLOCS['freetype'] = "#{ularch}/libfreetype.so.6"
  SOLOCS['fontconfig'] = "#{ularch}/libfontconfig.so.1"
  SOLOCS['pixman'] = "#{ularch}/libpixman-1.so.0"
  SOLOCS['gobject'] = "#{ularch}/libgobject-2.0.so.0"
  SOLOCS['glib'] = "#{larch}/libglib-2.0.so.0"
end
