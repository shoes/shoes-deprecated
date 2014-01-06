require File.expand_path('make/make')
extend Make
# set the absolute path to the deps library
XDEPABS = '/home/ccoupe/Projects/xmingw'

# set the relative path to deps directory
XDEP = "../../xmingw"


# set a prefix path to where your cross compiler and tools live
XTOOLS = '/usr/bin/i586-mingw32msvc-'
# set CC to your mingw cross compiler using XTOOLS.
CC = "#{XTOOLS}cc"

# set where the the dependcies .h are 
XINC = "#{XDEP}/include"

# set where the dependent libs live
XLIB = "#{XDEP}/lib"


# sadly, Ruby includes and libs are in 1.9.1 even for later 1.9.3
# define where the ruby includes are for the cross compiled Ruby.
XRUBYINC = "#{XINC}/ruby-1.9.1"
# and where does the "cross installed" Ruby stdlib (bigdecimal.rb)
# live - It need to be an absolute path, not relative. 
XRUBYLIB = "#{XDEPABS}/lib/ruby/1.9.1"
# Where are the Wine includes (for 'winhttp.h') 
#XWINH = "/usr/include/wine/windows"
XWINH = "#{XDEP}/wineh"
#  get the rbconfig of the cross compiled ruby
#  FIXME. This won't work for ruby 1.9.3 hosts
#require "#{XRUBYLIB}/i386-mingw32/rbconfig.rb"


file_list = ["shoes/*.c"] + %w{shoes/native/windows.c shoes/http/winhttp.c shoes/http/windownload.c}

# Building with curl doesn't work yet.
#file_list = ["shoes/*.c"] + %w{shoes/native/windows.c shoes/http/curl.c}
  
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = ["shoes/appwin32.o"]

# Linux build environment for MingW
CAIRO_CFLAGS = "-I#{XINC}/glib-2.0 -I#{XLIB}/glib-2.0/include -I#{XINC}/cairo"
CAIRO_LIB = '-lcairo'
PANGO_CFLAGS = "-I#{XINC}/pango-1.0"
PANGO_LIB = '-lpangocairo-1.0 -lpango-1.0 -lpangoft2-1.0 -lpangowin32-1.0'
LINUX_CFLAGS = "-I#{XINC} -I#{XRUBYINC} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"
LINUX_CFLAGS << " -I#{XRUBYINC}/i386-mingw32 -I#{XWINH}"
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
LINUX_CFLAGS << " -DXMD_H -DHAVE_BOOLEAN -DSHOES_WIN32 -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -DCOBJMACROS"
LINUX_LDFLAGS = " -DBUILD_DLL -L#{XLIB} -lungif -ljpeg -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgmodule-2.0 -lgthread-2.0 -fPIC -shared"
LINUX_LDFLAGS << ' --enable-auto-import -lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lole32 -loleaut32 -ladvapi32 -loleacc'

cp APP['icons']['win32'], "shoes/appwin32.ico"
  
LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CAIRO_LIB} #{PANGO_LIB} -L#{XDEP}/bin -lmsvcrt-ruby191 #{XDEP}/lib/winhttp.a" 

