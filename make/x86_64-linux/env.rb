#
# Build a 64 bit Linux Tight Shoes (from a 64 bit host)
# In this case Unbuntu 13.10 to debian 7.2 in a chroot.
# Uses a ruby built from source and a minimal curl. 
# You should modify this to build 
# 
#ENV['DEBUG'] = "true" # turns on the tracing log
#ENV['GTK'] = "gtk+-3.0" # pick this or "gtk+-2.0"
ENV['GTK'] = "gtk+-2.0"
# I don't recommend try to copy Gtk2 -it only works mysteriously
COPY_GTK = false 
ENV['GDB'] = "SureYouBetcha" # compile -g,  strip symbols when nil
# CHROOT = "/srv/chroot/deb386"
CHROOT = ""
# Where does ruby code live?
EXT_RUBY = "/usr/local"
SHOES_TGT_ARCH = 'x86_64-linux'# Specify where the Target system binaries live. 
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
pkggtk ="#{ularch}/pkgconfig/#{ENV['GTK']}.pc" 
# Use Ruby or curl for downloads
RUBY_HTTP = true
if !RUBY_HTTP
# where does your cross compiled Curl live? Don't depend on the hosts
# curl -
  curlloc = "#{uldir}"
  CURL_LDFLAGS = `pkg-config --libs #{curlloc}/pkgconfig/libcurl.pc`.strip
  CURL_CFLAGS = `pkg-config --cflags #{curlloc}/pkgconfig/libcurl.pc`.strip
  file_list = %w{shoes/native/gtk.c shoes/http/curl.c} + ["shoes/*.c"]
else
  file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]
end
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
LINUX_CFLAGS << " -DSHOES_GTK -fPIC" 
LINUX_CFLAGS << " -DGTK3" unless ENV['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << " #{CURL_CFLAGS if !RUBY_HTTP}  -I#{TGT_SYS_DIR}usr/include "
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
#LINUX_LDFLAGS << " #{CURL_LDFLAGS}"
#LINUX_LDFLAGS << " -L. -rdynamic -Wl,-export-dynamic"


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
if ENV['GTK'] == 'gtk+-2.0' && COPY_GTK == true
  SOLOCS['gtk2'] = "#{ularch}/libgtk-x11-2.0.so"
  SOLOCS['gdk2'] = "#{ularch}/libgdk-x11-2.0.so"
  SOLOCS['atk'] = "#{ularch}/libatk-1.0.so.0"
  SOLOCS['gio'] = "#{ularch}/libgio-2.0.so.0"
  SOLOCS['pangoft2'] = "#{ularch}/libpangoft2-1.0.so"
  SOLOCS['pangocairo'] = "#{ularch}/libpangocairo-1.0.so"
  SOLOCS['gdk_pixbuf'] = "#{ularch}/libgdk_pixbuf-2.0.so"
  SOLOCS['cairo'] = "#{ularch}/libcairo.so"
  SOLOCS['pango'] = "#{ularch}/libpango-1.0.so"
  SOLOCS['freetype'] = "#{ularch}/libfreetype.so"
  SOLOCS['fontconfig'] = "#{ularch}/libfontconfig.so"
  SOLOCS['gobject'] = "#{ularch}/libgobject-2.0.so.0"
  SOLOCS['glib'] = "#{larch}/libglib-2.0.so.0"
  SOLOCS['ffi'] = "#{ularch}/libffi.so.5"
end
