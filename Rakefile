require 'rake'
require 'rake/clean'
require 'platform/skel'
require 'fileutils'
include FileUtils

APPNAME = ENV['APPNAME'] || "Shoes"
RELEASE_ID, RELEASE_NAME = 2, "Raisins"
NAME = APPNAME.downcase.gsub(/\W+/, '')
SONAME = 'shoes'
REVISION = `git rev-list HEAD`.split.length + 1
VERS = ENV['VERSION'] || "0.r#{REVISION}"
PKG = "#{NAME}-#{VERS}"
APPARGS = ENV['APPARGS']
FLAGS = %w[DEBUG VIDEO]

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
CLEAN.include ["{bin,shoes}/#{BIN}", "dist"]

# Guess the environment
unless ENV['MSVC'] or ENV['DDKBUILDENV']
  if ENV['MSVCDir']
    ENV['MSVC'] = ENV['MSVCDir']
  elsif ENV['VS71COMNTOOLS']
    ENV['MSVC'] = File.expand_path("../../Vc7", ENV['VS71COMNTOOLS'])
  elsif ENV['VCToolkitInstallDir']
    ENV['MSVC'] = ENV['VCToolkitInstallDir']
  end
end

# Check the environment
def env(x)
  unless ENV[x]
    abort "Your #{x} environment variable is not set!"
  end
  ENV[x]
end

def rm_svn(root)
  require 'find'
  Find.find(root) do |path|
    if File.basename(path) == '.svn'
      rm_rf path
      Find.prune
    end
  end
end

# Subs in special variables
def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
  File.open(after, 'w') do |a|
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(reg) do
          if reg2.include? '\1'
            reg2.gsub(%r!\\1!, Object.const_get($1))
          else
            reg2
          end
        end
      end
    end
  end
end

ruby_so = Config::CONFIG['RUBY_SO_NAME']
ext_ruby = "deps/ruby"
unless File.exists? ext_ruby
  ext_ruby = Config::CONFIG['prefix']
end

desc "Same as `rake build'"
task :default => [:build]

desc "Does a full compile, with installer"
task :package => [:build, :installer]

task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << %{#define SHOES_RELEASE_ID #{RELEASE_ID}\n#define SHOES_RELEASE_NAME "#{RELEASE_NAME}"\n#define SHOES_REVISION #{REVISION}\n}
  end
end

task "dist/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION})\n}
  end
end

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  mkdir_p "dist/ruby"
  cp_r  "#{ext_ruby}/lib/ruby/1.8", "dist/ruby/lib"
  FileList["rubygems/*"].each do |rg|
    cp_r  rg, "dist/ruby/lib"
  end
  rm_f  FileList["dist/ruby/lib/**/*.h"]
  unless ENV['STANDARD']
    %w[cgi cgi.rb cgi-lib.rb rdoc rss shell soap webrick wsdl xsd].each do |libn|
      rm_rf "dist/ruby/lib/#{libn}"
    end
  end
  # %w[superredcloth].each do |libn| # hpricot sqlite3-ruby
  #   gem = Gem.cache.find_name(libn).last
  #   cp_r "#{gem.full_gem_path}/lib", "dist/ruby"
  # end
  case PLATFORM when /win32/
    cp FileList["#{ext_ruby}/bin/*"], "dist/"
    cp FileList["deps/cairo/bin/*"], "dist/"
    cp FileList["deps/pango/bin/*"], "dist/"
    if ENV['VIDEO']
      cp_r FileList["deps/vlc/bin/*"], "dist/"
    end
  when /darwin/
    if ENV['SHOES_DEPS_PATH']
      dylibs = %w[lib/libcairo.2.dylib lib/libcairo.2.dylib lib/libgmodule-2.0.0.dylib lib/libintl.8.dylib lib/libruby.dylib
         lib/libglib-2.0.0.dylib lib/libgobject-2.0.0.dylib lib/libpng12.0.dylib lib/libpango-1.0.0.dylib 
         lib/pango/1.6.0/modules/pango-basic-atsui.la lib/libpangocairo-1.0.0.dylib 
         lib/pango/1.6.0/modules/pango-basic-atsui.so etc/pango/pango.modules
         lib/pango/1.6.0/modules/pango-arabic-lang.so lib/pango/1.6.0/modules/pango-arabic-lang.la
         lib/pango/1.6.0/modules/pango-indic-lang.so lib/pango/1.6.0/modules/pango-indic-lang.la
         lib/libjpeg.62.dylib lib/libungif.4.dylib]
      if ENV['VIDEO']
        dylibs.push *%w[lib/liba52.0.dylib lib/libfaac.0.dylib lib/libfaad.0.dylib lib/libmp3lame.0.dylib
          lib/libvorbis.0.dylib lib/libogg.0.dylib
          lib/libvorbisenc.2.dylib lib/libpostproc.dylib lib/libavformat.dylib lib/libavcodec.dylib lib/libavutil.dylib]
      end
      dylibs.each do |libn|
        cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
      end.each do |libn|
        next unless libn =~ %r!^lib/(.+?\.dylib)$!
        libf = $1
        sh "install_name_tool -id /tmp/dep/#{libn} dist/#{libf}"
        ['dist/shoes-bin', *Dir['dist/*.dylib']].each do |lib2|
          sh "install_name_tool -change /tmp/dep/#{libn} @executable_path/#{libf} #{lib2}"
        end
      end
      if ENV['VIDEO']
        mkdir_p "dist/plugins"
        sh "cp -r deps/lib/vlc/**/*.dylib dist/plugins"
        sh "strip -x dist/*.dylib"
        sh "strip -x dist/plugins/*.dylib"
        sh "strip -x dist/ruby/lib/**/*.bundle"
      end
    end
  else
    cp    "#{ext_ruby}/lib/lib#{ruby_so}.so", "dist"
    ln_s  "lib#{ruby_so}.so", "dist/libruby.so.1.8"
  end

  cp_r  "lib", "dist/lib"
  cp_r  "samples", "dist/samples"
  cp_r  "static", "dist/static"
  rm_svn "dist"
  cp    "README", "dist/README.txt"
  cp    "CHANGELOG", "dist/CHANGELOG.txt"
  cp    "COPYING", "dist/COPYING.txt"
  
  case PLATFORM when /darwin/
    rm_rf "#{APPNAME}.app"
    mkdir "#{APPNAME}.app"
    mkdir "#{APPNAME}.app/Contents"
    cp_r "dist", "#{APPNAME}.app/Contents/MacOS"
    mkdir "#{APPNAME}.app/Contents/Resources"
    mkdir "#{APPNAME}.app/Contents/Resources/English.lproj"
    sh "ditto platform/mac/Shoes.icns #{APPNAME}.app/"
    sh "ditto platform/mac/Shoes.icns #{APPNAME}.app/Contents/Resources/"
    rewrite "platform/mac/Info.plist", "#{APPNAME}.app/Contents/Info.plist"
    cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
    cp "platform/mac/pangorc", "#{APPNAME}.app/Contents/MacOS/"
    cp "platform/mac/command-manual.rb", "#{APPNAME}.app/Contents/MacOS/"
    rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/shoes-launch"
    chmod 0755, "#{APPNAME}.app/Contents/MacOS/shoes-launch"
    rewrite "platform/mac/shoes", "#{APPNAME}.app/Contents/MacOS/shoes"
    chmod 0755, "#{APPNAME}.app/Contents/MacOS/shoes"
    # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
    `echo -n 'APPL????' > "#{APPNAME}.app/Contents/PkgInfo"`
  when /win32/
    cp "platform/msw/shoes.exe.manifest", "dist/#{NAME}.exe.manifest"
    cp "dist/zlib1.dll", "dist/zlib.dll"
  end
end

# use the platform Ruby claims
case PLATFORM
when /win32/
  SRC = FileList["shoes/*.c"] 
  OBJ = SRC.map do |x|
    x.gsub(/\.c$/, '.obj')
  end

  # MSVC build environment
  MSVC_LIBS = %[msvcrt-ruby18.lib pango-1.0.lib pangocairo-1.0.lib gobject-2.0.lib glib-2.0.lib cairo.lib giflib.lib jpeg.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib advapi32.lib oleacc.lib]
  MSVC_LIBS << " libvlc.lib" if ENV['VIDEO']
  MSVC_LIBS2 = ""
  MSVC_LIBS2 << " bufferoverflowu.lib" if ENV['DDKBUILDENV']
  MSVC_LIBS << MSVC_LIBS2

  MSVC_CFLAGS = %q[/ML /DWIN32 /DSHOES_WIN32 /DWIN32_LEAN_AND_MEAN
    /Ideps\vlc\include
    /Ideps\cairo\include
    /Ideps\cairo\include\cairo
    /Ideps\pango\include\pango-1.0
    /Ideps\pango\include\glib-2.0
    /Ideps\pango\lib\glib-2.0\include
    /Ideps\ruby\lib\ruby\1.8\i386-mswin32
    /Ideps\curl\include
    /I.
    /O2 /GR /EHsc
  ].gsub(/\n\s*/, ' ')

  MSVC_LDFLAGS = "/NOLOGO"

  FLAGS.each do |flag|
    MSVC_CFLAGS << " /D#{flag}" if ENV[flag]
  end
  if ENV['DEBUG']
    MSVC_CFLAGS << " /Zi"
    MSVC_LDFLAGS << " /DEBUG"
  end
  MSVC_CFLAGS << " /I#{ENV['CRT_INC_PATH']}" if ENV['CRT_INC_PATH']
  MSVC_LDFLAGS << " /LIBPATH:#{ENV['CRT_LIB_PATH'][0..-2]}\i386" if ENV['CRT_LIB_PATH']
  MSVC_LDFLAGS << " /LIBPATH:#{ENV['SDK_LIB_PATH'][0..-2]}\i386" if ENV['SDK_LIB_PATH']

  # MSVC build tasks
  task :build_os => [:buildenv_win32, :build_skel, "dist/#{NAME}.exe"]

  task :buildenv_win32 do
    unless ENV['DDKBUILDENV']
      vcvars32_bat = File.join(env('MSVC'), "vcvars32.bat")
      unless File.exists?(vcvars32_bat)
        vcvars32_bat = File.join(env('MSVC'), "bin/vcvars32.bat")
      end
      `"#{vcvars32_bat}" x86 && set`.each do |line|
        if line =~ /(\w+)=(.+)/
          ENV[$1] = $2
        end
      end
    end
    mkdir_p "dist"
  end

  task "dist/#{NAME}.exe" => ["dist/lib#{SONAME}.dll", "bin/main.obj", "shoes/appwin32.res"] do |t|
    rm_f t.name
    sh "link #{MSVC_LDFLAGS} /OUT:#{t.name} /LIBPATH:dist " +
      "/SUBSYSTEM:WINDOWS bin/main.obj shoes/appwin32.res lib#{SONAME}.lib #{MSVC_LIBS2}"
  end

  task "dist/pull.exe" => ["bin/pull.obj"] do |t|
    rm_f t.name
    sh "link #{MSVC_LDFLAGS} /OUT:#{t.name} /LIBPATH:deps/curl/lib " +
      "/SUBSYSTEM:WINDOWS bin/pull.obj libcurl.lib #{MSVC_LIBS2}"
  end

  task "dist/lib#{SONAME}.dll" => ["shoes/version.h"] + OBJ do |t|
    sh "link #{MSVC_LDFLAGS} /OUT:#{t.name} /dll " +
      "/LIBPATH:#{ext_ruby}/lib " +
      "/LIBPATH:deps/cairo/lib " +
      "/LIBPATH:deps/pango/lib " +
      "/LIBPATH:deps/vlc/lib #{OBJ.join(' ')} #{MSVC_LIBS}"
  end

  rule ".obj" => ".c" do |t|
    sh "cl /c /nologo /TP /Fo#{t.name} #{MSVC_CFLAGS} #{t.source}"
  end

  rule ".res" => ".rc" do |t|
    sh "rc /Fo#{t.name} #{t.source}"
  end

  task :installer do
    mkdir_p "pkg"
    rm_rf "dist/nsis"
    cp_r  "platform/msw", "dist/nsis"
    cp "shoes/appwin32.ico", "dist/nsis/setup.ico"
    rewrite "dist/nsis/base.nsi", "dist/nsis/#{NAME}.nsi"
    Dir.chdir("dist/nsis") do
      sh "\"#{env('NSIS')}\\makensis.exe\" #{NAME}.nsi"
    end
    mv "dist/nsis/#{PKG}.exe", "pkg"
  end
else
  require 'rbconfig'

  CC = "gcc"
  SRC = FileList["shoes/*.{c}"]
  OBJ = SRC.map do |x|
    x.gsub(/\.\w+$/, '.o')
  end

  # Linux build environment
  CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || `pkg-config --cflags cairo`.strip
  CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : `pkg-config --libs cairo`.strip
  PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || `pkg-config --cflags pango`.strip
  PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : `pkg-config --libs pango`.strip

  LINUX_CFLAGS = %[-I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{Config::CONFIG['archdir']}]
  LINUX_LIB_NAMES = %W[#{ruby_so} cairo pangocairo-1.0 ungif]
  FLAGS.each do |flag|
    LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
  end
  if ENV['DEBUG']
    LINUX_CFLAGS << " -g"
  end

  case PLATFORM when /darwin/
    DLEXT = "dylib"
    LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{Config::CONFIG["CFLAGS"]}"
    LINUX_LDFLAGS = "-framework Carbon -dynamiclib -Wl,-single_module #{Config::CONFIG["LDFLAGS"]} INSTALL_NAME"
    LINUX_LIB_NAMES << 'jpeg.62'
    if ENV['VIDEO']
      LINUX_LDFLAGS << " ./deps/lib/libvlc.a  ./deps/lib/vlc/libmemcpymmx.a ./deps/lib/vlc/libi420_rgb_mmx.a ./deps/lib/vlc/libi422_yuy2_mmx.a ./deps/lib/vlc/libi420_ymga_mmx.a ./deps/lib/vlc/libi420_yuy2_mmx.a ./deps/lib/vlc/libmemcpymmxext.a ./deps/lib/vlc/libmemcpy3dn.a ./deps/lib/vlc/libffmpeg.a ./deps/lib/vlc/libstream_out_switcher.a ./deps/lib/vlc/libquicktime.a ./deps/lib/vlc/libxvideo.a ./deps/lib/vlc/libauhal.a ./deps/lib/vlc/libmacosx.a -framework vecLib -lpthread -lm -liconv -lintl -liconv -lc -lpostproc -lavformat -lavcodec -lz -la52 -lfaac -lfaad -lmp3lame -lx264 -lxvidcore -lvorbisenc -lavutil -lvorbis -lm -logg -lm -lavformat -lavcodec -lz -la52 -lfaac -lfaad -lmp3lame -lx264 -lxvidcore -lvorbisenc -lavutil -lvorbis -lm -logg -framework QuickTime -framework Carbon -lm -lXxf86vm -lXinerama -L/usr/X11R6/lib -lSM -lICE -lX11 -lXext -lXv -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework IOKit -framework Cocoa -framework Carbon -framework QuickTime -lobjc -ObjC -framework OpenGL -framework AGL -read_only_relocs suppress"
    end
    if ENV['UNIVERSAL']
      LINUX_CFLAGS << " -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc"
      LINUX_LDFLAGS << " -arch i386 -arch ppc"
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
    elsif ENV['PPC']
      LINUX_CFLAGS << " -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
      LINUX_LDFLAGS << " -arch ppc"
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
    end
  else
    DLEXT = "so"
    LINUX_CFLAGS << " -DSHOES_GTK -fPIC #{`pkg-config --cflags gtk+-2.0`.strip}"
    LINUX_LDFLAGS =" #{`pkg-config --libs gtk+-2.0`.strip} -fPIC -shared"
    LINUX_LIB_NAMES << 'jpeg'
    LINUX_LIB_NAMES << "vlc" if ENV['VIDEO']
  end
  LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

  task :build_os => [:buildenv_linux, :build_skel, "dist/#{NAME}"]

  task :buildenv_linux do
    rm_rf "dist"
    mkdir_p "dist"
  end

  LINUX_LIBS << " -L#{Config::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB}"

  task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] do |t|
    bin = "#{t.name}-bin"
    rm_f t.name
    rm_f bin
    sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']}"
    if PLATFORM !~ /darwin/
      rewrite "platform/nix/shoes.launch", t.name, %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'LD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} $@' >> #{t.name}} 
      chmod 0755, t.name
    end
  end

  task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
    ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
    sh "#{CC} -o #{t.name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    case PLATFORM when /darwin/
      %w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
        sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{t.name}"
      end
    end
  end

  rule ".o" => ".mm" do |t|
    sh "#{CC} -c -o#{t.name} -fpascal-strings -Wundef -Wno-ctor-dtor-privacy -fno-strict-aliasing -fno-common #{t.source}"
  end

  rule ".o" => ".c" do |t|
    sh "#{CC} -I. -c #{LINUX_CFLAGS} #{t.source}"
    mv File.basename(t.name), t.name
  end

  case PLATFORM when /darwin/
    task :installer do
      mkdir_p "pkg"
      rm_rf "dmg"
      mkdir_p "dmg"
      cp_r "#{APPNAME}.app", "dmg"
      mv "dmg/#{APPNAME}.app/Contents/MacOS/samples", "dmg/samples"
      ln_s "/Applications", "dmg/Applications"
      sh "DYLD_LIBRARY_PATH= platform/mac/pkg-dmg --target pkg/#{PKG}.dmg --source dmg --volname '#{APPNAME}' --license COPYING --copy platform/mac/dmg_ds_store:/.DS_Store --mkdir /.background --copy platform/mac/shoes-dmg.jpg:/.background" # --format UDRW"
      rm_rf "dmg"
    end
  end
end

task :tarball => ['bin/main.c', 'shoes/version.h'] do
  mkdir_p "pkg"
  rm_rf PKG
  sh "svn export . #{PKG}"
  sh "svn export rubygems #{PKG}/rubygems"
  rm "#{PKG}/bin/main.skel"
  rm "#{PKG}/Rakefile"
  rm "#{PKG}/use-deps"
  cp "bin/main.c", "#{PKG}/bin/main.c"
  cp "shoes/version.h", "#{PKG}/shoes/version.h"
  rewrite "platform/nix/Makefile", "#{PKG}/Makefile", /^(REVISION) = .+?$/, 'REVISION = \1'
  sh "tar czvf pkg/#{PKG}.tar.gz #{PKG}"
  rm_rf PKG
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end
