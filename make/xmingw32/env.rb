#
# Build shoes for Windows/Gtk[2|3]  Ruby is cross compiled
# The Gtk3 headers/libs are already rewritten so pkg-config should
# be ok. Ruby might need rewrites. 
# Curl is not used on Windows (thank god)
# It's not really a chroot - it only looks like one and Gtk2/3 is different
# Remember, on Windows the dlls are in bin/ and usr/[include|bin] doesn't
# exist. Nor do the {arch} directories. 
#ENV['DEBUG'] = "true" # turns on the tracing log
#ENV['GTK'] = "gtk+-3.0" # pick this or "gtk+-2.0"
ENV['GTK'] = "gtk+-2.0"
COPY_GTK = true
ENV['GDB'] = "basic" # 'basic' = keep symbols,  or 'profile'
if ENV['GTK'] == "gtk+-2.0"
  CHROOT = "/srv/chroot/mingwgtk2"
else
  CHROOT = "/srv/chroot/mingw32"
end
# Where does ruby code live? Please cross compile Ruby. 
# Use ruby 2.1.0
EXT_RUBY = "/srv/chroot/mingwgtk2/opt/local"
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
STRIP = "i686-w64-mingw32-strip -x"
WINDRES = "i686-w64-mingw32-windres"
# These ENV vars are used by the extconf.rb files (and tasks.rb)
ENV['SYSROOT']=CHROOT
ENV['CC']=CC
ENV['TGT_RUBY_PATH']=EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
#ENV['TGT_RUBY_V'] = '1.9.1'
ENV['TGT_RUBY_V'] = '2.1.0'
TGT_RUBY_V = ENV['TGT_RUBY_V'] 
#ENV['TGT_RUBY_SO'] = "msvcrt-ruby191"
ENV['TGT_RUBY_SO'] = "msvcrt-ruby210"
EXT_RBCONFIG = "#{EXT_RUBY}/lib/ruby/#{TGT_RUBY_V}/#{SHOES_TGT_ARCH}/rbconfig.rb"
ENV['EXT_RBCONFIG'] = EXT_RBCONFIG 
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.1.pc"
pkggtk ="#{uldir}/pkgconfig/#{ENV['GTK']}.pc" 
# winhttp or RUBY?
RUBY_HTTP = true

ENV['PKG_CONFIG_PATH'] = "#{ularch}/pkgconfig"
WINVERSION = "#{REVISION}#{TINYVER}-#{ENV['GTK']=='Gtk+-3.0' ? 'gtk3' : 'gtk2'}-32"
WINFNAME = "#{APPNAME}-#{WINVERSION}"
if RUBY_HTTP
  file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]
else
 file_list = %w{shoes/native/gtk.c shoes/http/winhttp.c shoes/http/windownload.c} + ["shoes/*.c"] 
end
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

LINUX_CFLAGS << " -DSHOES_GTK -DSHOES_GTK_WIN32 "
LINUX_CFLAGS << "-DRUBY_HTTP " if RUBY_HTTP
LINUX_CFLAGS << "-DGTK3 " unless ENV['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << xfixrvmp(`pkg-config --cflags "#{pkgruby}"`.strip)+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/#{arch} "
LINUX_CFLAGS << xfixip("-I/usr/include")+" "
LINUX_CFLAGS << xfixip(`pkg-config --cflags "#{pkggtk}"`.strip)+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/local/include "
if ENV['GDB']== 'profile'
  LINUX_CFLAGS <<  '-pg'
end
LINUX_CFLAGS << " -Wno-unused-but-set-variable "
LINUX_CFLAGS << " -mms-bitfields -D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS "

#LINUX_CFLAGS << " #{CAIRO_CFLAGS} #{PANGO_CFLAGS} "
 
# I don't think the line below belongs in this file. 
cp APP['icons']['win32'], "shoes/appwin32.ico"

LINUX_LIB_NAMES = %W[gif-4 jpeg]

DLEXT = "dll"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
LINUX_LDFLAGS << "-lfontconfig" if ENV['GTK'] == 'gtk+-2.0'

# dont use the ruby link info
RUBY_LDFLAGS = "-Wl,-export-all-symbols "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lmsvcrt-ruby210 "

LINUX_LDFLAGS << " -pg -lwinhttp -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 "

#LINUX_LIBS = " -L/usr/lib "
LINUX_LIBS = " -L#{bindll} "
LINUX_LIBS << LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} "

# This could be precomputed by rake linux:setup:xxx 
# but for now make a hash of all the dep libs that need to be copied.
# and their location. This should be used in pre_build instead of 
# copy_deps_to_dist, although either could work. 
# Reference: http://www.gtk.org/download/win32_contentlist.php
SOLOCS = {}
#SOLOCS['ruby'] = "#{EXT_RUBY}/bin/msvcrt-ruby191.dll"
SOLOCS['ruby'] = "#{EXT_RUBY}/bin/msvcrt-ruby210.dll"
#SOLOCS['curl'] = "#{curlloc}/bin/libcurl-4.dll"
#SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{bindll}/libgif-4.dll"
SOLOCS['jpeg'] = "#{bindll}/libjpeg-9.dll"
SOLOCS['libyaml'] = "#{bindll}/libyaml-0-2.dll"
SOLOCS['intl'] = "#{bindll}/intl.dll"
SOLOCS['iconv'] = "#{bindll}/libiconv-2.dll"
SOLOCS['ffi'] = "#{bindll}/libffi-5.dll"
SOLOCS['eay'] = "#{bindll}/libeay32.dll"
SOLOCS['gdbm'] = "#{bindll}/libgdbm-3.dll"
SOLOCS['gdbmc'] = "#{bindll}/libgdbm_compat-3.dll"
SOLOCS['ssl'] = "#{bindll}/ssleay32.dll"
SOLOCS['sqlite'] = "#{bindll}/sqlite3.dll"
if ENV['GTK'] == 'gtk+-3.0' && COPY_GTK == true
  SOLOCS['atk'] = "#{bindll}/libatk-1.0-0.dll"
  SOLOCS['cairo'] = "#{bindll}/libcairo-2.dll"
  SOLOCS['cairo-gobj'] = "#{bindll}/libcairo-gobject-2.dll"
  SOLOCS['ffi'] = "#{bindll}/libffi-6.dll"
  SOLOCS['fontconfig'] = "#{bindll}/libfontconfig-1.dll"
  SOLOCS['freetype'] = "#{bindll}/libfreetype-6.dll"
  SOLOCS['gdkpixbuf'] = "#{bindll}/libgdk_pixbuf-2.0-0.dll"
  SOLOCS['gdk3'] = "#{bindll}/libgdk-3-0.dll"
  SOLOCS['gio'] = "#{bindll}/libgio-2.0-0.dll"
  SOLOCS['glib'] = "#{bindll}/libglib-2.0-0.dll"
  SOLOCS['gmodule'] = "#{bindll}/libgmodule-2.0-0.dll"
  SOLOCS['gobject'] = "#{bindll}/libgobject-2.0-0.dll"
  SOLOCS['gtk3'] = "#{bindll}/libgtk-3-0.dll"
  SOLOCS['iconv'] = "#{bindll}/libiconv-2.dll"
  SOLOCS['intl8'] = "#{bindll}/libintl-8.dll"
  SOLOCS['pango'] = "#{bindll}/libpango-1.0-0.dll"
  SOLOCS['pangocairo'] = "#{bindll}/libpangocairo-1.0-0.dll"
  SOLOCS['pangoft'] = "#{bindll}/libpangoft2-1.0-0.dll"
  SOLOCS['pango32'] = "#{bindll}/libpangowin32-1.0-0.dll"
  SOLOCS['pixman'] = "#{bindll}/libpixman-1-0.dll"
  SOLOCS['png15'] = "#{bindll}/libpng15-15.dll"
  SOLOCS['xml2'] = "#{bindll}/libxml2-2.dll"
  SOLOCS['pthread'] = "#{bindll}/pthreadGC2.dll"
  SOLOCS['zlib1'] = "#{bindll}/zlib1.dll"
  SOLOCS['lzma'] = "#{bindll}/liblzma-5.dll"
  SOLOCS['pthreadGC2'] = "#{bindll}/pthreadGC2.dll"   # GTK3 
  SOLOCS['pthread'] = "/usr/i686-w64-mingw32/lib/libwinpthread-1.dll" # Ruby
end
if ENV['GTK'] == 'gtk+-2.0' && COPY_GTK == true
  SOLOCS['atk'] = "#{bindll}/libatk-1.0-0.dll"
  SOLOCS['cairo'] = "#{bindll}/libcairo-2.dll"
  SOLOCS['cairo-gobj'] = "#{bindll}/libcairo-gobject-2.dll"
#  SOLOCS['ffi'] = "#{bindll}/libffi-6.dll"
  SOLOCS['fontconfig'] = "#{bindll}/libfontconfig-1.dll"
  SOLOCS['freetype'] = "#{bindll}/freetype6.dll"
  SOLOCS['gdkpixbuf'] = "#{bindll}/libgdk_pixbuf-2.0-0.dll"
  SOLOCS['gdk2'] = "#{bindll}/libgdk-win32-2.0-0.dll"
  SOLOCS['gio'] = "#{bindll}/libgio-2.0-0.dll"
  SOLOCS['glib'] = "#{bindll}/libglib-2.0-0.dll"
  SOLOCS['gmodule'] = "#{bindll}/libgmodule-2.0-0.dll"
  SOLOCS['gobject'] = "#{bindll}/libgobject-2.0-0.dll"
  SOLOCS['gtk2'] = "#{bindll}/libgtk-win32-2.0-0.dll"
#  SOLOCS['iconv'] = "#{bindll}/libiconv-2.dll"
  SOLOCS['intl'] = "#{bindll}/intl.dll"
  SOLOCS['pango'] = "#{bindll}/libpango-1.0-0.dll"
  SOLOCS['pangocairo'] = "#{bindll}/libpangocairo-1.0-0.dll"
  SOLOCS['pangoft'] = "#{bindll}/libpangoft2-1.0-0.dll"
  SOLOCS['pango32'] = "#{bindll}/libpangowin32-1.0-0.dll"
  SOLOCS['pixman'] = "#{bindll}/libgdk_pixbuf-2.0-0.dll"
  SOLOCS['png14'] = "#{bindll}/libpng14-14.dll"
  SOLOCS['xml2'] = "#{bindll}/libexpat-1.dll"
  SOLOCS['thread'] = "#{bindll}/libgthread-2.0-0.dll"
  SOLOCS['zlib1'] = "#{bindll}/zlib1.dll"
#  SOLOCS['lzma'] = "#{bindll}/liblzma-5.dll"
#  SOLOCS['pthreadGC2'] = "#{bindll}/pthreadGC2.dll"
  SOLOCS['siji'] = "/usr/lib/gcc/i686-w64-mingw32/4.8/libgcc_s_sjlj-1.dll"
  SOLOCS['pthread'] = "/usr/i686-w64-mingw32/lib/libwinpthread-1.dll"
end
