require "devkit"
# define where your deps are
ShoesDeps = "E:/shoesdeps/mingw"
SHOES_TGT_ARCH = 'i386-mingw32'
APP['GTK'] = "gtk+-2.0"
#ENV['GDB'] = "basic" # 'basic' = keep symbols,  or 'profile'
WINVERSION = "#{APP['VERSION']}-#{APP['GTK']=='Gtk+-3.0' ? 'gtk3' : 'gtk2'}-w32"
WINFNAME = "#{APPNAME}-#{WINVERSION}"
WIN32_CFLAGS = []
WIN32_LDFLAGS = []
WIN32_LIBS = []

EXT_RUBY = RbConfig::CONFIG["prefix"]

SRC = FileList[*%w{shoes/native/gtk.c shoes/http/rbload.c shoes/*.c}]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

DLEXT = "dll"
ADD_DLL = []

CC = "i686-w64-mingw32-gcc"
ENV['CC'] = CC		# for building sqlite3 gem
ENV['ShoesDeps'] = ShoesDeps # also for sqlite3 gem
STRIP = "i686-w64-mingw32-strip -x"
STRIP = "strip -x"
WINDRES = "windres"
PKG_CONFIG = "#{ShoesDeps}/bin/pkg-config"  # the one from glib

if ENV['DEBUG'] || ENV['GDB']
  WIN32_CFLAGS << "-g -O0"
else
  WIN32_CFLAGS << "-O -Wall"
end

GTK_CFLAGS = `#{PKG_CONFIG} --cflags gtk+-2.0`.chomp
GTK_LDFLAGS = `#{PKG_CONFIG} --libs gtk+-2.0`.chomp
CAIRO_CFLAGS = `#{PKG_CONFIG} --cflags glib-2.0`.chomp + 
                  `#{PKG_CONFIG} --cflags cairo`.chomp
CAIRO_LDFLAGS = `#{PKG_CONFIG} --libs cairo`.chomp
PANGO_CFLAGS = `#{PKG_CONFIG} --cflags pango`.chomp
PANGO_LDFLAGS = `#{PKG_CONFIG} --libs pango`.chomp

RUBY_LDFLAGS = "-L#{RbConfig::CONFIG["bindir"]} #{RbConfig::CONFIG["LIBRUBYARG"]} "
RUBY_LDFLAGS << "-Wl,-export-all-symbols "
#RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lmsvcrt-ruby210 "

WIN32_CFLAGS << "-DSHOES_GTK -DSHOES_GTK_WIN32 -DRUBY_HTTP"
WIN32_CFLAGS << "-Wno-unused-but-set-variable"
WIN32_CFLAGS << "-D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS"
WIN32_CFLAGS << GTK_CFLAGS
WIN32_CFLAGS << CAIRO_CFLAGS
WIN32_CFLAGS << PANGO_CFLAGS
RbConfig::CONFIG.select { |k, _| k[/hdrdir/] }.each_key do |v|
   WIN32_CFLAGS << "-I#{RbConfig::CONFIG[v]}"
end
#["jpeg-6b-4-lib", "giflib-4.1.4-1-lib"].each { |n|
#   WIN32_CFLAGS << "-Isandbox/#{n}/include"
#   WIN32_LDFLAGS << "-Lsandbox/#{n}/lib"
#}
WIN32_CFLAGS << "-Ishoes"

WIN32_LDFLAGS << "-lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32"
WIN32_LDFLAGS << "-lgif -ljpeg -lfontconfig"
WIN32_LDFLAGS << "-L#{ENV['RI_DEVKIT']}/mingw/bin".gsub('\\','/').gsub(/^\//,'//')
WIN32_LDFLAGS << "-lwinpthread-1 -fPIC -shared"
WIN32_LDFLAGS << GTK_LDFLAGS
WIN32_LDFLAGS << CAIRO_LDFLAGS
WIN32_LDFLAGS << PANGO_LDFLAGS
WIN32_LDFLAGS << RUBY_LDFLAGS

#WIN32_LIBS = WIN32_LDFLAGS
#WIN32_LIBS << "-L#{ENV['RI_DEVKIT']}/mingw/bin".gsub('\\','/').gsub(/^\//,'//')
#WIN32_LIBS << "-lgif -ljpeg -Wl,-export-all-symbols -lmsvcrt-ruby210 -lcairo -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl "
WIN32_LIBS << RUBY_LDFLAGS
WIN32_LIBS << CAIRO_LDFLAGS
WIN32_LIBS << PANGO_LDFLAGS

# Cleaning up duplicates. Clunky? Hell yes!
wIN32_CFLAGS = WIN32_CFLAGS.join(' ').split(' ').uniq
wIN32_LDFLAGS = WIN32_LDFLAGS.join(' ').split(' ').uniq
wIN32_LIBS = WIN32_LIBS.join(' ').split(' ').uniq

LINUX_CFLAGS = wIN32_CFLAGS.join(' ')
LINUX_LDFLAGS = wIN32_LDFLAGS.join(' ')
LINUX_LIBS = wIN32_LIBS.join(' ')

# hash of dlls to copy in tasks.rb pre_build
bindll = "#{ShoesDeps}/bin"
rubydll = "#{EXT_RUBY}/bin"
devdll = "#{ENV['RI_DEVKIT']}/mingw/bin"
SOLOCS = {
  'ruby'    => "#{rubydll}/msvcrt-ruby210.dll",
  'gif'     => "#{bindll}/libgif-4.dll",
  'jpeg'    => "#{bindll}/libjpeg-9.dll",
  'libyaml' => "#{rubydll}/libyaml-0-2.dll",
  'intl'    => "#{rubydll}/intl.dll",
  'iconv'   => "#{rubydll}/libiconv-2.dll",
  'ffi'     => "#{rubydll}/libffi-6.dll",
  'eay'     => "#{rubydll}/libeay32.dll",
  'gdbm'    => "#{rubydll}/libgdbm-3.dll",
  'gdbmc'   => "#{rubydll}/libgdbm_compat-3.dll",
  'ssl'     => "#{rubydll}/ssleay32.dll",
  'sqlite'  => "#{bindll}/sqlite3.dll"
}
SOLOCS.merge!(
  {
    'atk'         => "#{bindll}/libatk-1.0-0.dll",
    'cairo'       => "#{bindll}/libcairo-2.dll",
    'cairo-gobj'  => "#{bindll}/libcairo-gobject-2.dll",
    'fontconfig'  => "#{bindll}/libfontconfig-1.dll",
    'freetype'    => "#{bindll}/freetype6.dll",
    'gdkpixbuf'   => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'gdk2'        => "#{bindll}/libgdk-win32-2.0-0.dll",
    'gio'         => "#{bindll}/libgio-2.0-0.dll",
    'glib'        => "#{bindll}/libglib-2.0-0.dll",
    'gmodule'     => "#{bindll}/libgmodule-2.0-0.dll",
    'gobject'     => "#{bindll}/libgobject-2.0-0.dll",
    'gtk2'        => "#{bindll}/libgtk-win32-2.0-0.dll",
    'intl'        => "#{bindll}/intl.dll",
    'pango'       => "#{bindll}/libpango-1.0-0.dll",
    'pangocairo'  => "#{bindll}/libpangocairo-1.0-0.dll",
    'pangoft'     => "#{bindll}/libpangoft2-1.0-0.dll",
    'pango32'     => "#{bindll}/libpangowin32-1.0-0.dll",
    'pixman'      => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'png14'       => "#{bindll}/libpng14-14.dll",
    'xml2'        => "#{bindll}/libexpat-1.dll",
    'thread'      => "#{bindll}/libgthread-2.0-0.dll",
    'zlib1'       => "#{bindll}/zlib1.dll",
    'pthread'     => "#{devdll}/libwinpthread-1.dll",
    'sjlj'        => "#{devdll}/libgcc_s_sjlj-1.dll"
  }
) if APP['GTK'] == 'gtk+-2.0'
