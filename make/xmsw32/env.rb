#
# Build shoes for Windows native GUI  Ruby is cross compiled
# Curl is not used on Windows (thank god) but might be needed to compile
# It's not really a chroot - it only looks like one. 
# Remember, on Windows the dlls are in bin/ and usr/[include|bin] doesn't
# exist. Nor do the {arch} directories. I'm reusing the chroot (aka deps)
# that I built for mingwgtk2

ENV['GDB'] = "SureYouBetcha" # compile -g,  strip symbols when nil
CHROOT = "/srv/chroot/mingwgtk2"
# Where does ruby code live? Please cross compile Ruby. 
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
WINDRES = "i686-w64-mingw32-windres"
# These ENV vars are used by the extconf.rb files (and tasks.rb)
ENV['SYSROOT']=CHROOT
ENV['CC']=CC
ENV['TGT_RUBY_PATH']=EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '1.9.1'
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-1.9.pc"
pkggtk ="#{uldir}/pkgconfig/#{ENV['GTK']}.pc" 
# where is curl (lib,include) Can be commented since we don't use curl
# for MinGW
#curlloc = "#{CHROOT}/usr/local"
#CURL_LDFLAGS = `pkg-config --libs #{curlloc}/lib/pkgconfig/libcurl.pc`.strip
#CURL_CFLAGS = `pkg-config --cflags #{curlloc}/lib/pkgconfig/libcurl.pc`.strip

ENV['PKG_CONFIG_PATH'] = "#{ularch}/pkgconfig"

file_list = %w{shoes/native/windows.c shoes/http/winhttp.c shoes/http/windownload.c} + ["shoes/*.c"] 

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

LINUX_CFLAGS << " -DXMD_H -DHAVE_BOOLEAN -DSHOES_WIN32 -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -DCOBJMACROS "
LINUX_CFLAGS << xfixrvmp(`pkg-config --cflags "#{pkgruby}"`.strip)+" "
LINUX_CFLAGS << "#{CAIRO_CFLAGS} #{PANGO_CFLAGS} "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/#{arch} "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}include "

#LINUX_CFLAGS << " #{CAIRO_CFLAGS} #{PANGO_CFLAGS} "
 
# I don't think the line below belongs in this file. 
cp APP['icons']['win32'], "shoes/appwin32.ico"

LINUX_LIB_NAMES = %W[gif jpeg]

DLEXT = "dll"
LINUX_LDFLAGS = " -DBUILD_DLL -L#{uldir} -lgif -ljpeg -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgmodule-2.0 -lgthread-2.0 -fPIC -shared"
LINUX_LDFLAGS << ' -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lole32 -loleaut32 -ladvapi32 -loleacc'
LINUX_LDFLAGS << ' -lpangocairo-1.0'

# dont use the ruby link info
RUBY_LDFLAGS = "-Wl,-export-all-symbols "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lmsvcrt-ruby191 "

LINUX_LDFLAGS << " -lpthread -lwinhttp -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 "

LINUX_LIBS = " -L/usr/lib "
LINUX_LIBS << LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} "

# This could be precomputed by rake linux:setup:xxx 
# but for now make a hash of all the dep libs that need to be copied.
# and their location. This should be used in pre_build instead of 
# copy_deps_to_dist, although either could work. 
# Reference: http://www.gtk.org/download/win32_contentlist.php
SOLOCS = {}
SOLOCS['ruby'] = "#{EXT_RUBY}/bin/msvcrt-ruby191.dll"
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

  SOLOCS['atk'] = "#{bindll}/libatk-1.0-0.dll"
  SOLOCS['cairo'] = "#{bindll}/libcairo-2.dll"
  SOLOCS['cairo-gobj'] = "#{bindll}/libcairo-gobject-2.dll"
#  SOLOCS['ffi'] = "#{bindll}/libffi-6.dll"
  SOLOCS['fontconfig'] = "#{bindll}/libfontconfig-1.dll"
  SOLOCS['freetype'] = "#{bindll}/freetype6.dll"
#  SOLOCS['gdkpixbuf'] = "#{bindll}/libgdk_pixbuf-2.0-0.dll"
#  SOLOCS['gdk2'] = "#{bindll}/libgdk-win32-2.0-0.dll"
  SOLOCS['gio'] = "#{bindll}/libgio-2.0-0.dll"
  SOLOCS['glib'] = "#{bindll}/libglib-2.0-0.dll"
  SOLOCS['gmodule'] = "#{bindll}/libgmodule-2.0-0.dll"
  SOLOCS['gobject'] = "#{bindll}/libgobject-2.0-0.dll"
#  SOLOCS['gtk2'] = "#{bindll}/libgtk-win32-2.0-0.dll"
#  SOLOCS['iconv'] = "#{bindll}/libiconv-2.dll"
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
  SOLOCS['pthread'] = "/usr/i686-w64-mingw32/lib/libwinpthread-1.dll"

