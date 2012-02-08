require File.expand_path('make/make')
extend Make

EXT_RUBY = "../mingw"

# use the platform Ruby claims
#require 'rbconfig'
#CC = ENV['CC'] ? ENV['CC'] : "gcc"

# set a prefix path to where your cross compiler and tools live
XTOOLS = '/usr/bin/i586-mingw32msvc-'
# set CC to your mingw cross compiler using XTOOLS.
CC = "#{XTOOLS}cc"

# set where your dependent code like cairo is kept. No slash at the end.
XDEP = "/home/ccoupe/Projects/xmingw"

# set where the .h are 
XINC = "#{XDEP}/include"
# set where the dll that Shoes doesn't compile live at.
XLIB = "#{XDEP}/lib"
# sadly, Ruby includes and libs are in 1.9.1 even for later 1.9.3
# define where the ruby includes are for the cross compiled Ruby.
XRUBYINC = "#{XINC}/ruby-1.9.1"
# Where the Wine .h files are
XWINH = "#{XDEP}/wineh"

#file_list = ["shoes/*.c"] + %w{shoes/native/windows.c shoes/http/winhttp.c shoes/http/windownload.c}
file_list = ["shoes/*.c"] + %w{shoes/native/windows.c shoes/http/curl.c}
  
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = ["shoes/appwin32.o"]

# Linux build environment for MingW
#CAIRO_CFLAGS = "-I/mingw/include/glib-2.0 -I/mingw/lib/glib-2.0/include -I/mingw/include/cairo"
CAIRO_CFLAGS = "-I/#{XINC}/glib-2.0 -I/#{XLIB}/glib-2.0/include -I#{XINC}/cairo"
CAIRO_LIB = '-lcairo'
PANGO_CFLAGS = "-I#{XINC}/pango-1.0"
PANGO_LIB = '-lpangocairo-1.0 -lpango-1.0 -lpangoft2-1.0 -lpangowin32-1.0'
LINUX_CFLAGS = "-I#{XRUBYINC} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"
LINUX_CFLAGS << " -I#{XRUBYINC}/i386-mingw32 -I#{XWINH}"
#LINUX_CFLAGS = %[-Wall -I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{Config::CONFIG['archdir']}]
#if Config::CONFIG['rubyhdrdir']
#  LINUX_CFLAGS << " -I#{Config::CONFIG['rubyhdrdir']} -I#{Config::CONFIG['rubyhdrdir']}/#{SHOES_RUBY_ARCH}"
#end
#LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 ungif]
LINUX_LIB_NAMES = %W[cairo pangocairo-1.0 ungif]

FLAGS.each do |flag|
  LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
end
if ENV['DEBUG']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9"

DLEXT = 'dll'
#LINUX_CFLAGS << ' -I. -I/mingw/include'
#LINUX_CFLAGS << ' -I/mingw/include/ruby-1.9.1/ruby'
LINUX_CFLAGS << " -DXMD_H -DHAVE_BOOLEAN -DSHOES_WIN32 -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -DCOBJMACROS"
LINUX_LDFLAGS = " -DBUILD_DLL -L#{XLIB} -lungif -ljpeg -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgmodule-2.0 -lgthread-2.0 -fPIC -shared"
LINUX_LDFLAGS << ' --enable-auto-import -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lole32 -loleaut32 -ladvapi32 -loleacc' # -lwinhttp' # problem expected winhttp

cp APP['icons']['win32'], "shoes/appwin32.ico"
  
LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

#LINUX_LIBS << " -L#{Config::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB}"
LINUX_LIBS << " #{CAIRO_LIB} #{PANGO_LIB} -L#{XDEP}/ruby-1.9.2-p290 -lmsvcrt-ruby191" 
