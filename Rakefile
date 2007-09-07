require 'rake'
require 'rake/clean'
require 'platform/skel'
require 'fileutils'
include FileUtils

APPNAME = ENV['APPNAME'] || "Shoes"
NAME = APPNAME.downcase.gsub(/\W+/, '')
SONAME = 'shoes'
VERS = ENV['VERSION'] || "0.1"
PKG = "#{NAME}-#{VERS}"
APPARGS = ENV['APPARGS']

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
CLEAN.include ["{bin,shoes}/#{BIN}", "dist"]

# Guess the environment
unless ENV['MSVC'] or ENV['DDKBUILDENV']
  if ENV['VS71COMNTOOLS']
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

# Subs in special variables
def rewrite before, after
  File.open(after, 'w') do |a|
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(/\#\{(\w+)\}/) { Object.const_get($1) }
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

desc "Does a full compile, for the OS you're running on"
task :build => :build_os do
  mkdir_p "dist/ruby"
  cp_r  "#{ext_ruby}/lib/ruby/1.8", "dist/ruby/lib"
  unless ENV['STANDARD']
    %w[rdoc rexml rss soap test webrick wsdl xsd].each do |libn|
      rm_rf "dist/ruby/lib/#{libn}"
    end
  end
  # %w[superredcloth].each do |libn| # hpricot sqlite3-ruby
  #   gem = Gem.cache.find_name(libn).last
  #   cp_r "#{gem.full_gem_path}/lib", "dist/ruby"
  # end
  case PLATFORM when /win32/
    cp    FileList["#{ext_ruby}/bin/*"], "dist/"
    cp    FileList["deps/cairo/bin/*"], "dist/"
    cp    FileList["deps/pango/bin/*"], "dist/"
  when /darwin/
    if ENV['SHOES_DEPS_PATH']
      %w[lib/libcairo.2.dylib lib/libcairo.2.dylib lib/libgmodule-2.0.0.dylib lib/libintl.8.dylib lib/libruby.dylib
         lib/libglib-2.0.0.dylib lib/libgobject-2.0.0.dylib lib/libpng12.0.dylib lib/libpango-1.0.0.dylib 
         lib/pango/1.6.0/modules/pango-basic-atsui.la lib/libpangocairo-1.0.0.dylib 
         lib/pango/1.6.0/modules/pango-basic-atsui.so etc/pango/pango.modules
         lib/libjpeg62.dylib lib/libungif.4.dylib].
      each do |libn|
        cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
      end
    end
  else
    cp    "#{ext_ruby}/lib/lib#{ruby_so}.so", "dist"
    ln_s  "lib#{ruby_so}.so", "dist/libruby.so.1.8"
  end

  cp_r  "lib", "dist/lib"
  rm_rf "dist/lib/.svn"
  cp_r  "samples", "dist/samples"
  rm_rf "dist/samples/.svn"
  cp_r  "static", "dist/static"
  rm_rf "dist/static/.svn"
  cp    "README.txt", "dist"
  
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
    rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/#{NAME}"
    chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}"
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
  SRC = FileList["shoes/*.{c,rc}"] 
  OBJ = SRC.map do |x|
    x.gsub(/\.c$/, '.obj').gsub(/\.rc$/, '.res')
  end

  # MSVC build environment
  MSVC_LIBS = %[msvcrt-ruby18.lib pango-1.0.lib pangocairo-1.0.lib gobject-2.0.lib glib-2.0.lib cairo.lib giflib.lib jpeg.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib advapi32.lib oleacc.lib]
  MSVC_LIBS << " bufferoverflowu.lib" if ENV['DDKBUILDENV']

  MSVC_CFLAGS = %q[/ML /DWIN32 /DSHOES_WIN32 /DWIN32_LEAN_AND_MEAN
    /Ideps\cairo\include
    /Ideps\cairo\include\cairo
    /Ideps\pango\include\pango-1.0
    /Ideps\pango\include\glib-2.0
    /Ideps\pango\lib\glib-2.0\include
    /Ideps\ruby\lib\ruby\1.8\i386-mswin32
    /I.
    /O2 /GR /EHsc
  ].gsub(/\n\s*/, ' ')

  MSVC_LDFLAGS = "/NOLOGO"

  MSVC_CFLAGS << " /DDEBUG" if ENV['DEBUG']
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

  task "dist/#{NAME}.exe" => OBJ + ["bin/main.obj"] do |t|
    rm_f t.name
    sh "link #{MSVC_LDFLAGS} /OUT:#{t.name} /LIBPATH:#{ext_ruby}/lib " +
      "/LIBPATH:deps/cairo/lib " +
      "/LIBPATH:deps/pango/lib " +
      "/SUBSYSTEM:WINDOWS #{OBJ.join(' ')} bin/main.obj #{MSVC_LIBS}"
  end

  rule ".obj" => ".c" do |t|
    sh "cl /c /nologo /TP /Fo#{t.name} #{MSVC_CFLAGS} #{t.source}"
  end

  rule ".res" => ".rc" do |t|
    sh "rc /Fo#{t.name} #{t.source}"
  end

  task :installer do
    mkdir_p "pkg"
    rm_rf "dist/installer"
    cp_r  "installer", "dist"
    File.open("dist/installer/#{NAME}.nsi", "w") do |f|
      File.foreach("installer/base.nsi") do |line|
        line.gsub!(/\#\{(\w+)\}/) { Object.const_get($1) }
        f << line
      end
    end
    Dir.chdir("dist/installer") do
      sh "\"#{env('NSIS')}\\makensis.exe\" #{NAME}.nsi"
    end
    mv "dist/installer/#{PKG}.exe", "pkg"
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
  if ENV['DEBUG']
    LINUX_CFLAGS << " -DDEBUG"
  end

  case PLATFORM when /darwin/
    DLEXT = "dylib"
    LINUX_CFLAGS << " -DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{Config::CONFIG["CFLAGS"]}"
    LINUX_LDFLAGS = "-framework Carbon -dynamiclib -Wl,-single_module #{Config::CONFIG["LDFLAGS"]} INSTALL_NAME"
    LINUX_LIB_NAMES << 'jpeg62'
  else
    DLEXT = "so"
    LINUX_CFLAGS << " -DSHOES_GTK #{`pkg-config --cflags gtk+-2.0`.strip}"
    LINUX_LDFLAGS =" #{`pkg-config --libs gtk+-2.0`.strip} -fPIC -shared"
    LINUX_LIB_NAMES << 'jpeg'
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
      sh %{echo 'APPPATH="${0%/*}"' > #{t.name}}
      sh %{echo 'LD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} $@' >> #{t.name}}
      chmod 0755, t.name
    end
  end

  task "dist/lib#{SONAME}.#{DLEXT}" => OBJ do |t|
    ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
    sh "#{CC} -o #{t.name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
  end

  rule ".o" => ".mm" do |t|
    sh "#{CC} -c -o#{t.name} -fpascal-strings -Wundef -Wno-ctor-dtor-privacy -fno-strict-aliasing -fno-common #{t.source}"
  end

  rule ".o" => ".c" do |t|
    sh "#{CC} -I. -c #{LINUX_CFLAGS} #{t.source}"
    mv File.basename(t.name), t.name
  end
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end
