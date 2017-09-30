# xmsys2 cross  build  
cf =(ENV['ENV_CUSTOM'] || "xmsys2-custom.yaml")
gtk_version = '3'
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  GtkDeps = custmz['GtkLoc'] ? custmz['GtkLoc'] : ShoesDeps
  ENABLE_MS_THEME = custmz['MS-Theme'] == true
  ENV['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  APP['VIDEO'] = true
  APP['GTK'] = 'gtk+-3.0'
  APP['INSTALLER'] = custmz['Installer'] == 'qtifw'? 'qtifw' : 'nsis'
  APP['INSTALLER_LOC'] = custmz['InstallerLoc']
else
  # define where your deps are
  #ShoesDeps = "E:/shoesdeps/mingw"
  ShoesDeps = "C:/Users/Cecil/sandbox"
  EXT_RUBY = RbConfig::CONFIG["prefix"]
  ENABLE_MS_THEME = false
end
puts "Ruby = #{EXT_RUBY} Deps = #{ShoesDeps}, Gtk: #{GtkDeps}"
SHOES_GEM_ARCH = "#{Gem::Platform.local}"
SHOES_TGT_ARCH = 'i386-mingw32'
WINVERSION = "#{APP['VERSION']}-gtk3-w32"
WINFNAME = "#{APPNAME}-#{WINVERSION}"
WIN32_CFLAGS = []
WIN32_LDFLAGS = []
WIN32_LIBS = []
RUBY_HTTP = true

file_list = []
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

DLEXT = "dll"
ADD_DLL = []

CC = "i686-w64-mingw32-gcc"
STRIP = "strip -x"
WINDRES = "i686-w64-mingw32-windres"
PKG_CONFIG = "pkg-config"

# dance on ENV['PKG_CONFIG_PATH'] We want something  pkg-config can use
ENV['PKG_CONFIG_PATH'] = "#{ShoesDeps}/lib/pkgconfig"
$stderr.puts "PKG PATH: #{ENV['PKG_CONFIG_PATH']}"
if APP['GDB']
  WIN32_CFLAGS << "-g3 -O0"
else
  WIN32_CFLAGS << "-O -Wall"
end

gtk_pkg_path = "#{GtkDeps}/lib/pkgconfig/gtk+-3.0.pc"

GTK_CFLAGS = `#{PKG_CONFIG} --cflags gtk+-3.0 --define-variable=prefix=#{ShoesDeps}`.chomp
GTK_LDFLAGS = `#{PKG_CONFIG} --libs gtk+-3.0 --define-variable=prefix=#{ShoesDeps}`.chomp
CAIRO_CFLAGS = `#{PKG_CONFIG} --cflags glib-2.0 --define-variable=prefix=#{ShoesDeps}`.chomp +
    `#{PKG_CONFIG} --cflags cairo --define-variable=prefix=#{ShoesDeps}`.chomp
CAIRO_LDFLAGS = `#{PKG_CONFIG} --libs cairo --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_CFLAGS = `#{PKG_CONFIG} --cflags pango --define-variable=prefix=#{ShoesDeps}`.chomp
PANGO_LDFLAGS = `#{PKG_CONFIG} --libs pango --define-variable=prefix=#{ShoesDeps}`.chomp

RUBY_LDFLAGS = " -Wl,-export-all-symbols -L#{EXT_RUBY}/lib -lmsvcrt-ruby220 "

WIN32_CFLAGS << "-DSHOES_GTK -DSHOES_GTK_WIN32 -DRUBY_HTTP -DVIDEO"
WIN32_CFLAGS << "-DGTK3 "
WIN32_CFLAGS << "-Wno-unused-but-set-variable"
WIN32_CFLAGS << "-D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS"
WIN32_CFLAGS << GTK_CFLAGS
WIN32_CFLAGS << CAIRO_CFLAGS
WIN32_CFLAGS << PANGO_CFLAGS
WIN32_CFLAGS << "-I#{ShoesDeps}/include/librsvg-2.0/librsvg "
WIN32_CFLAGS << `pkg-config --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-2.2.pc --define-variable=prefix=#{EXT_RUBY}`.chomp
WIN32_CFLAGS << "-Ishoes"

WIN32_LDFLAGS << "-lshell32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32"
WIN32_LDFLAGS << "-lgif -ljpeg -lfontconfig"
WIN32_LDFLAGS << "-L#{ENV['RI_DEVKIT']}/mingw/bin".gsub('\\','/').gsub(/^\//,'//')
WIN32_LDFLAGS << "-fPIC -shared"
WIN32_LDFLAGS << GTK_LDFLAGS
WIN32_LDFLAGS << CAIRO_LDFLAGS
WIN32_LDFLAGS << PANGO_LDFLAGS
WIN32_LDFLAGS << RUBY_LDFLAGS

WIN32_LIBS << RUBY_LDFLAGS
WIN32_LIBS << CAIRO_LDFLAGS
WIN32_LIBS << PANGO_LDFLAGS
WIN32_LIBS << "-L#{ShoesDeps}/lib -lrsvg-2"

# Cleaning up duplicates. Clunky? Hell yes!
wIN32_CFLAGS = WIN32_CFLAGS.join(' ').split(' ').uniq
wIN32_LDFLAGS = WIN32_LDFLAGS.join(' ').split(' ').uniq
wIN32_LIBS = WIN32_LIBS.join(' ').split(' ').uniq

LINUX_CFLAGS = wIN32_CFLAGS.join(' ')
LINUX_LDFLAGS = wIN32_LDFLAGS.join(' ')
LINUX_LIBS = wIN32_LIBS.join(' ')

# hash of dlls to copy in tasks.rb pre_build - no particular order.
bindll = "#{ShoesDeps}/bin"
basedll = "#{ShoesDeps}/../basedll"
gtkdll = "#{GtkDeps}/bin"
SOLOCS = {
  'ruby'    => "#{EXT_RUBY}/bin/msvcrt-ruby220.dll",
  'gif'     => "#{bindll}/libgif-7.dll",
  'jpeg'    => "#{bindll}/libjpeg-9.dll",
  'libyaml' => "#{bindll}/libyaml-0-2.dll",
  'iconv'   => "#{bindll}/libiconv-2.dll",
  'eay'     => "#{bindll}/libeay32.dll",
  'gdbm'    => "#{bindll}/libgdbm-4.dll",
  'ssl'     => "#{bindll}/ssleay32.dll",
  'gmp'     => "#{basedll}/libgmp-10.dll", # ruby 2.2.6 needs this
  'gcc-dw'  => "#{basedll}/libgcc_s_dw2-1.dll",
  'sqlite'  => "#{bindll}/libsqlite3-0.dll"
}


SOLOCS.merge!(
  {
    'atk'         => "#{bindll}/libatk-1.0-0.dll",
    'cairo'       => "#{bindll}/libcairo-2.dll",
    'cairo-gobj'  => "#{bindll}/libcairo-gobject-2.dll",
    'ffi'         => "#{bindll}/libffi-6.dll",
    'fontconfig'  => "#{bindll}/libfontconfig-1.dll",
    'freetype'    => "#{bindll}/libfreetype-6.dll",
    'gdkpixbuf'   => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'gio'         => "#{bindll}/libgio-2.0-0.dll",
    'glib'        => "#{bindll}/libglib-2.0-0.dll",
    'gmodule'     => "#{bindll}/libgmodule-2.0-0.dll",
    'gobject'     => "#{bindll}/libgobject-2.0-0.dll",
    'gdk3'        => "#{gtkdll}/libgdk-3-0.dll", 
    'gtk3'        => "#{gtkdll}/libgtk-3-0.dll",
    'pixman'      => "#{bindll}/libpixman-1-0.dll", 
    'intl8'       => "#{bindll}/libintl-8.dll",
    'pango'       => "#{bindll}/libpango-1.0-0.dll",
    'pangocairo'  => "#{bindll}/libpangocairo-1.0-0.dll",
    'pangoft'     => "#{bindll}/libpangoft2-1.0-0.dll",
    'pango32'     => "#{bindll}/libpangowin32-1.0-0.dll",
    'pixbuf'      => "#{bindll}/libgdk_pixbuf-2.0-0.dll",
    'harfbuzz'    => "#{bindll}/libharfbuzz-0.dll",
    'png16'       => "#{bindll}/libpng16-16.dll",
    'croco'       => "#{bindll}/libcroco-0.6-3.dll",
    'rsvg'        => "#{bindll}/librsvg-2-2.dll",
    'xml2'        => "#{bindll}/libxml2-2.dll",
    'thread'      => "#{bindll}/libgthread-2.0-0.dll",
    'zlib1'       => "#{bindll}/zlib1.dll",
    'pthread'     => "#{basedll}/libwinpthread-1.dll",
  }
)

