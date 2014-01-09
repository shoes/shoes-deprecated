#
# Build shoes for Windows/Gtk3  Ruby is cross compiled
# The Gtk3 headers/libs are already rewritten so pkg-config should
# be ok. Ruby might need rewrites. 
# Curl is also separately compiled and located.
# It's not reall a chroot - it only looks like one.
# Remember, on Windows the dlls are in bin/ and usr/[include|bin] doesn't
# exist. Nor does the {arch} directories. 
#ENV['DEBUG'] = "true" # turns on the tracing log
#ENV['GTK'] = "gtk+-3.0" # pick this or "gtk+-2.0"
ENV['GTK'] = "gtk+-3.0"
COPY_GTK = true
ENV['GDB'] = "SureYouBetcha" # compile -g,  strip symbols when nil
CHROOT = "/srv/chroot/mingw32"
# Where does ruby code live? 
EXT_RUBY = "#{CHROOT}/usr/local"
SHOES_TGT_ARCH = "i386-mingw32"
# Specify where the Target system binaries live. 
# Trailing slash is important.
TGT_SYS_DIR = "#{CHROOT}/"
# Setup some shortcuts for the library locations. 
# These are not ruby paths. 
# depends on what ruby was compiled to produce. Don't guess. 
arch = 'i386-mingw32'
uldir = "#{TGT_SYS_DIR}lib"
ularch = "#{TGT_SYS_DIR}lib"
larch = "#{TGT_SYS_DIR}lib/"
bindll = "#{TGT_SYS_DIR}bin"
ulbin = "#{TGT_SYS_DIR}usr/local/bin"
# Set appropriately (in my PATH, or use abs)
CC = "i686-w64-mingw32-gcc"
# These ENV vars are used by the extconf.rb files (and tasks.rb)
ENV['SYSROOT']=CHROOT
ENV['CC']=CC
ENV['TGT_RUBY_PATH']=EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '1.9.1'
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-1.9.pc"
pkggtk ="#{uldir}/pkgconfig/#{ENV['GTK']}.pc" 
# where is curl (lib,include)
# Actually, we use curl. 
curlloc = "#{CHROOT}/usr/local"
CURL_LDFLAGS = `pkg-config --libs #{curlloc}/lib/pkgconfig/libcurl.pc`.strip
CURL_CFLAGS = `pkg-config --cflags #{curlloc}/lib/pkgconfig/libcurl.pc`.strip

ENV['PKG_CONFIG_PATH'] = "#{ularch}/pkgconfig"

file_list = %w{shoes/native/gtk.c shoes/http/winhttp.c shoes/http/windownload.c} + ["shoes/*.c"] 

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Hand code for your situation and Ruby purity. Mine is
# "Church of Whatever Works That I Can Understand"
def xfixip(path)
   path.gsub!(/-I\/usr\//, "-I#{TGT_SYS_DIR}usr/")
   return path
end

def xfixrvmp(path)
  #puts "path  in: #{path}"
  path.gsub!(/-I\/home\/ccoupe\/\.rvm/, "-I/home/cross/armv6-pi/rvm")
  path.gsub!(/-I\/usr\/local\//, "-I/#{TGT_SYS_DIR}usr/local/")
  #puts "path out: #{path}"
  return path
end

#  fix up the -L paths for rvm ruby. Undo when not using an rvm ruby
def xfixrvml(path)
  #path.gsub!(/-L\/home\/ccoupe\/\.rvm/, "-L#{TGT_SYS_DIR}rvm")
  return path
end

# fixup the -L paths for gtk and other libs
def xfixil(path) 
  path.gsub!(/-L\/usr\/lib/, "-L#{TGT_SYS_DIR}usr/lib")
  return path
end

# Target environment
CAIRO_CFLAGS = `pkg-config --cflags "#{ularch}/pkgconfig/cairo.pc"`.strip
CAIRO_LIB = `pkg-config --libs "#{ularch}/pkgconfig/cairo.pc"`.strip
PANGO_CFLAGS = `pkg-config --cflags "#{ularch}/pkgconfig/pango.pc"`.strip
PANGO_LIB = `pkg-config --libs "#{ularch}/pkgconfig/pango.pc"`.strip

png_lib = 'png'

if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end

LINUX_CFLAGS << " -DSHOES_GTK -DSHOES_GTK_WIN32"
LINUX_CFLAGS << " -DGTK3 " unless ENV['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << xfixrvmp(`pkg-config --cflags "#{pkgruby}"`.strip)+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/#{arch} "
LINUX_CFLAGS << xfixip("-I/usr/include")+" "
LINUX_CFLAGS << xfixip(`pkg-config --cflags "#{pkggtk}"`.strip)+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/local/include "

#LINUX_CFLAGS << " #{CAIRO_CFLAGS} #{PANGO_CFLAGS} "
 



LINUX_LIB_NAMES = %W[gif jpeg]

DLEXT = "dll"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# dont use the ruby link info
RUBY_LDFLAGS = "-Wl,-export-all-symbols "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lmsvcrt-ruby191 "

LINUX_LDFLAGS << " -lwinhttp -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 "

LINUX_LIBS = " -L/usr/lib "
LINUX_LIBS << LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} "

# This could be precomputed by rake linux:setup:xxx 
# but for now make a hash of all the dep libs that need to be copied.
# and their location. This should be used in pre_build instead of 
# copy_deps_to_dist, although either would work. 

SOLOCS = {}
SOLOCS['ruby'] = "#{EXT_RUBY}/bin/msvcrt-ruby191.dll"
SOLOCS['curl'] = "#{curlloc}/bin/libcurl-4.dll"
#SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{ulbin}/libgif-4.dll"
SOLOCS['jpeg'] = "#{bindll}/libjpeg-9.dll"
SOLOCS['libyaml'] = "#{bindll}/libyaml-0-2.dll"
if ENV['GTK'] == 'gtk+-2.0' && COPY_GTK == true
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
