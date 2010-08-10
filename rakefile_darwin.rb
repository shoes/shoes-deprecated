# for Mac
def copy_ext xdir, libdir
  Dir.chdir(xdir) do
    `ruby extconf.rb; make`
  end
  copy_files "#{xdir}/*.bundle", libdir
end

EXT_RUBY = "deps/ruby"
unless File.exists? EXT_RUBY
  EXT_RUBY = Config::CONFIG['prefix']
end

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  common_build

  if ENV['SHOES_DEPS_PATH']
    dylibs = IO.readlines("make/darwin/dylibs.shoes")
    if ENV['VIDEO']
      dylibs += IO.readlines("make/darwin/dylibs.video")
    end
    dylibs.each do |libn|
      cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
    end.each do |libn|
      next unless libn =~ %r!^lib/(.+?\.dylib)$!
      libf = $1
      sh "install_name_tool -id /tmp/dep/#{libn} dist/#{libf}"
      ["dist/#{NAME}-bin", *Dir['dist/*.dylib']].each do |lib2|
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

  if ENV['APP']
    if APP['clone']
      sh APP['clone'].gsub(/^git /, "#{GIT} --git-dir=#{ENV['APP']}/.git ")
    else
      cp_r ENV['APP'], "dist/app"
    end
    if APP['ignore']
      APP['ignore'].each do |nn|
        rm_rf "dist/app/#{nn}"
      end
    end
  end
  
  cp_r  "fonts", "dist/fonts"
  cp_r  "lib", "dist/lib"
  cp_r  "samples", "dist/samples"
  cp_r  "static", "dist/static"
  cp    "README", "dist/README.txt"
  cp    "CHANGELOG", "dist/CHANGELOG.txt"
  cp    "COPYING", "dist/COPYING.txt"
  
  rm_rf "#{APPNAME}.app"
  mkdir "#{APPNAME}.app"
  mkdir "#{APPNAME}.app/Contents"
  cp_r "dist", "#{APPNAME}.app/Contents/MacOS"
  mkdir "#{APPNAME}.app/Contents/Resources"
  mkdir "#{APPNAME}.app/Contents/Resources/English.lproj"
  sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/App.icns\""
  sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/Contents/Resources/App.icns\""
  rewrite "platform/mac/Info.plist", "#{APPNAME}.app/Contents/Info.plist"
  cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
  rewrite "platform/mac/pangorc", "#{APPNAME}.app/Contents/MacOS/pangorc"
  cp "platform/mac/command-manual.rb", "#{APPNAME}.app/Contents/MacOS/"
  rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
  chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
  chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
  rewrite "platform/mac/shoes", "#{APPNAME}.app/Contents/MacOS/#{NAME}"
  chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}"
  # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
  `echo -n 'APPL????' > "#{APPNAME}.app/Contents/PkgInfo"`
end

# use the platform Ruby claims
require 'rbconfig'

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list = ["shoes/*.c"] + %w{shoes/native/cocoa.m shoes/http/nsurl.m}

SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

# Linux build environment
CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || `pkg-config --cflags cairo`.strip
CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : `pkg-config --libs cairo`.strip
PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || `pkg-config --cflags pango`.strip
PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : `pkg-config --libs pango`.strip
png_lib = 'png'

LINUX_CFLAGS = %[-Wall -I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{VLC_CFLAGS} -I#{Config::CONFIG['archdir']}]
if Config::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{Config::CONFIG['rubyhdrdir']} -I#{Config::CONFIG['rubyhdrdir']}/#{RUBY_PLATFORM}"
end
  
LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 gif]

FLAGS.each do |flag|
  LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
end

if ENV['DEBUG']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9" if RUBY_1_9

DLEXT = "dylib"
LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{Config::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
LINUX_LDFLAGS = "-framework Cocoa -framework Carbon -dynamiclib -Wl,-single_module #{Config::CONFIG["LDFLAGS"]} INSTALL_NAME"
LINUX_LIB_NAMES << 'pixman-1' << 'jpeg.8'

if ENV['VIDEO']
  if VLC_0_8
    LINUX_CFLAGS << " -DVLC_0_8"
  end
  LINUX_LDFLAGS << " ./deps/lib/libvlc.a  ./deps/lib/vlc/libmemcpymmx.a ./deps/lib/vlc/libi420_rgb_mmx.a ./deps/lib/vlc/libi422_yuy2_mmx.a ./deps/lib/vlc/libi420_ymga_mmx.a ./deps/lib/vlc/libi420_yuy2_mmx.a ./deps/lib/vlc/libmemcpymmxext.a ./deps/lib/vlc/libmemcpy3dn.a ./deps/lib/vlc/libffmpeg.a ./deps/lib/vlc/libstream_out_switcher.a ./deps/lib/vlc/libquicktime.a ./deps/lib/vlc/libauhal.a ./deps/lib/vlc/libmacosx.a -framework vecLib -lpthread -lm -liconv -lintl -liconv -lc -lpostproc -lavformat -lavcodec -lz -la52 -lfaac -lfaad -lmp3lame -lx264 -lxvidcore -lvorbisenc -lavutil -lvorbis -lm -logg -lm -lavformat -lavcodec -lz -la52 -lfaac -lfaad -lmp3lame -lx264 -lxvidcore -lvorbisenc -lavutil -lvorbis -lm -logg -framework QuickTime -lm -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework IOKit -lobjc -ObjC -framework OpenGL -framework AGL -read_only_relocs suppress"
end
if ENV['UNIVERSAL']
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc"
  LINUX_LDFLAGS << " -arch i386 -arch ppc"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
elsif ENV['PPC']
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
  LINUX_LDFLAGS << " -arch ppc"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
else
  LINUX_CFLAGS << " -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64"
  LINUX_LDFLAGS << " -arch x86_64"
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
end

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

task :build_os => [:buildenv_linux, :build_skel, "dist/#{NAME}"]

task :buildenv_linux do
  unless ENV['VIDEO']
    rm_rf "dist"
    mkdir_p "dist"
  end
end

LINUX_LIBS << " -L#{Config::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB} #{VLC_LIB}"

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] do |t|
  bin = "#{t.name}-bin"
  rm_f t.name
  rm_f bin
  if RUBY_PLATFORM =~ /darwin/
    sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes -arch x86_64"
  else  
    sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']}"
    rewrite "platform/nix/shoes.launch", t.name, %r!/shoes-bin!, "/#{NAME}-bin"
    sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{t.name}}
    chmod 0755, t.name
  end
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
  sh "#{CC} -o #{t.name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
  %w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
    sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{t.name}"
  end
end

rule ".o" => ".m" do |t|
  sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
end

rule ".o" => ".c" do |t|
  sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end

task :stub do
  ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
  sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
end

task :installer do
  dmg_ds, dmg_jpg = "platform/mac/dmg_ds_store", "static/shoes-dmg.jpg"
  if APP['dmg']
    dmg_ds, dmg_jpg = APP['dmg']['ds_store'], APP['dmg']['background']
  end

  mkdir_p "pkg"
  rm_rf "dmg"
  mkdir_p "dmg"
  cp_r "#{APPNAME}.app", "dmg"
  unless ENV['APP']
    mv "dmg/#{APPNAME}.app/Contents/MacOS/samples", "dmg/samples"
  end
  ln_s "/Applications", "dmg/Applications"
  sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}"
  sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-bin"
  sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-launch"
  sh "DYLD_LIBRARY_PATH= platform/mac/pkg-dmg --target pkg/#{PKG}.dmg --source dmg --volname '#{APPNAME}' --copy #{dmg_ds}:/.DS_Store --mkdir /.background --copy #{dmg_jpg}:/.background" # --format UDRW"
  rm_rf "dmg"
end
