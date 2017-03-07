# msys2 native build 
# NOTE this can be used with older version of GTK3 than msys2 normally
# installs 
cf =(ENV['ENV_CUSTOM'] || "msys2-custom.yaml")
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
#APP['GTK'] = "gtk+-#{gtk_version}.0"
#ENV['GDB'] = "basic" # 'basic' = keep symbols,  or 'profile'
WINVERSION = "#{APP['VERSION']}-#{APP['GTK']=='gtk+-3.0' ? 'gtk3' : 'gtk2'}-w32"
WINFNAME = "#{APPNAME}-#{WINVERSION}"
WIN32_CFLAGS = []
WIN32_LDFLAGS = []
WIN32_LIBS = []
RUBY_HTTP = true
gtk_extra_list = []
if APP['GTK'] == "gtk+-3.0"
  gtk_extra_list = %w(shoes/native/gtkfixedalt.c shoes/native/gtkentryalt.c
               shoes/native/gtkcomboboxtextalt.c shoes/native/gtkbuttonalt.c
               shoes/native/gtkscrolledwindowalt.c shoes/native/gtkprogressbaralt.c )
end
if RUBY_HTTP
  file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + gtk_extra_list + ["shoes/*.c"] +
     ["shoes/plot/*.c"] + ["shoes/types/*.c"]
else
  file_list = %w{shoes/native/gtk.c shoes/http/winhttp.c shoes/http/windownload.c} + ["shoes/*.c"] 
end
file_list << "shoes/video/video.c" if APP['VIDEO']
SRC = FileList[*file_list]

OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

DLEXT = "dll"
ADD_DLL = []

CC = "gcc"
ENV['CC'] = CC		# for building sqlite3 gem
ENV['ShoesDeps'] = ShoesDeps # also for sqlite3 gem
STRIP = "strip -s"
WINDRES = "windres"
#PKG_CONFIG = "C:/msys64/usr/bin/pkg-config.exe"  # Has to be Ruby `path`
PKG_CONFIG = "pkg-config.exe"
ENV['PKG_CONFIG_PATH'] = ENV['PKG_CONFIG_PATH'].split(";").collect { |n| `cygpath -u #{n}`.chomp }.join(":")
#PKG_LOC = "C:/msys64/mingw32/lib/pkgconfig"      #   ditto
# dance on ENV['PKG_CONFIG_PATH'] We want something  pkg-config can use
#ENV['PKG_CONFIG_PATH'] = '/c/msys64/mingw32/lib/pkgconfig'+':'+ENV['PKG_CONFIG_PATH']
#$stderr.puts "have #{ENV['PKG_CONFIG_PATH']}"
if ENV['DEBUG'] || ENV['GDB']
  WIN32_CFLAGS << "-g3 -O0"
else
  WIN32_CFLAGS << "-O -Wall"
end

# fixup include paths
def xfixip(path)
   #path.gsub!(/-I\/mingw32\//, "-IC:/msys64/mingw32/")
   path.gsub!(/-I\/mingw32\//,  "-I#{`cygpath -m /mingw32/`.chomp}")
   return path
end
# fixup link paths
def xfixlp(path)
  #path.gsub!(/-L\/mingw32\//, "-LC:/msys64/mingw32/")
  path.gsub!(/-L\/mingw32\//,  "-L#{`cygpath -m /mingw32/`.chomp}")
  return path
end

#fixup ruby includes 
def xfixri(path)
  #path.gsub!(/-I\/usr\/local\//, "-I/#{TGT_SYS_DIR}usr/local/")
  return path
end

gtk_pkg_path = "#{GtkDeps}/lib/pkgconfig/gtk+-3.0.pc"
GTK_CFLAGS = xfixip(`#{PKG_CONFIG} --cflags #{gtk_pkg_path}`.chomp)
GTK_LDFLAGS = xfixlp(`#{PKG_CONFIG}  --libs gtk+-3.0`.chomp)
CAIRO_CFLAGS = xfixip(`#{PKG_CONFIG} --cflags glib-2.0`.chomp + 
                  `#{PKG_CONFIG} --cflags cairo`.chomp)
CAIRO_LDFLAGS = xfixlp(`#{PKG_CONFIG} --libs cairo`.chomp)
PANGO_CFLAGS = xfixip(`#{PKG_CONFIG} --cflags pango`.chomp)
PANGO_LDFLAGS = xfixlp(`#{PKG_CONFIG} --libs pango`.chomp)

RUBY_LDFLAGS = "-L#{RbConfig::CONFIG["bindir"]} #{RbConfig::CONFIG["LIBRUBYARG"]} "
RUBY_LDFLAGS << "-Wl,-export-all-symbols "
#RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lmsvcrt-ruby210 "

WIN32_CFLAGS << "-DSHOES_GTK -DSHOES_GTK_WIN32 -DRUBY_HTTP -DVIDEO"
WIN32_CFLAGS << "-DGTK3 "
WIN32_CFLAGS << "-Wno-unused-but-set-variable"
WIN32_CFLAGS << "-D__MINGW_USE_VC2005_COMPAT -DXMD_H -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -DCOBJMACROS"
WIN32_CFLAGS << GTK_CFLAGS
WIN32_CFLAGS << CAIRO_CFLAGS
WIN32_CFLAGS << PANGO_CFLAGS
WIN32_CFLAGS << "-I#{ShoesDeps}/include/librsvg-2.0/librsvg "
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
#WIN32_LDFLAGS << "-lwinpthread-1 -fPIC -shared"
WIN32_LDFLAGS << "-fPIC -shared"
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
WIN32_LIBS << "-L#{ShoesDeps}/lib -lrsvg-2"

# Cleaning up duplicates. Clunky? Hell yes!
wIN32_CFLAGS = WIN32_CFLAGS.join(' ').split(' ').uniq
wIN32_LDFLAGS = WIN32_LDFLAGS.join(' ').split(' ').uniq
wIN32_LIBS = WIN32_LIBS.join(' ').split(' ').uniq

LINUX_CFLAGS = wIN32_CFLAGS.join(' ')
LINUX_LDFLAGS = wIN32_LDFLAGS.join(' ')
LINUX_LIBS = wIN32_LIBS.join(' ')

# hash of dlls to copy in tasks.rb pre_build
bindll = "#{ShoesDeps}/bin"
#rubydll = "#{EXT_RUBY}/bin"
#devdll = "#{ENV['RI_DEVKIT']}/mingw/bin"
#libdll = "#{ShoesDeps}/lib"
# msys2 want's some things from or maybe its Ruby 2.2.6?
#basedll = "C:/msys64/mingw32/bin"
basedll = `cygpath -m /mingw32/bin`.chomp
gtkdll = "#{GtkDeps}/bin"
SOLOCS = {
  'ruby'    => "#{EXT_RUBY}/bin/msvcrt-ruby220.dll",
  #'gif'     => "#{bindll}/libgif-4.dll",
  'gif'     => "#{bindll}/libgif-7.dll",
  #'jpeg'    => "#{bindll}/libjpeg-8.dll",
  'jpeg'    => "#{bindll}/libjpeg-9.dll",
  'libyaml' => "#{bindll}/libyaml-0-2.dll",
  'iconv'   => "#{bindll}/libiconv-2.dll",
  'eay'     => "#{bindll}/libeay32.dll",
  'gdbm'    => "#{bindll}/libgdbm-4.dll",
  #'gdbm'    => "#{basedll}/libgdbm-4.dll",
  'ssl'     => "#{bindll}/ssleay32.dll",
  'gmp'     => "#{basedll}/libgmp-10.dll", # ruby 2.2.6 needs this
#  'gcc-dw'  => "#{basedll}/libgcc_s_dw2-1.dll",
  'gcc-dw'  => "#{basedll}/libgcc_s_dw2-1.dll",
  'sqlite'  => "#{bindll}/sqlite3.dll"
  #'sqlite'  => "#{`cygpath -m /mingw32/lib`.chomp}/sqlite3.13.0/sqlite3130.dll"
}

if APP['GTK'] == 'gtk+-3.0'
  SOLOCS.merge!(
    {
      'atk'         => "#{bindll}/libatk-1.0-0.dll",
      'cairo'       => "#{bindll}/libcairo-2.dll",
      'cairo-gobj'  => "#{bindll}/libcairo-gobject-2.dll",
      'ffi'         => "#{bindll}/libffi-6.dll",
      'fontconfig'  => "#{bindll}/libfontconfig-1.dll",
       #'expat'       => "#{bindll}/libexpat-1.dll",
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
      'pthread'     => "#{bindll}/libwinpthread-1.dll",
      #'pthread'     => "#{basedll}/libwinpthread-1.dll",
      'sjlj'        => "#{bindll}/libgcc_s_sjlj-1.dll" 
    }
  )
end

   
