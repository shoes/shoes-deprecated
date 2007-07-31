require 'rake'
require 'rake/clean'
require 'fileutils'
include FileUtils

NAME = "shoes"
APPNAME = "Shoes"
VERS = "0.1"
PKG = "#{NAME}-#{VERS}"

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"

CLEAN.include ["{bin,shoes}/#{BIN}", "dist"]

# Guess the environment
unless ENV['MSVC']
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

ruby_so = Config::CONFIG['RUBY_SO_NAME']
ext_ruby = "external/ruby"
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
    cp    FileList["external/cairo/bin/*"], "dist/"
    cp    FileList["external/pango/bin/*"], "dist/"
  when /darwin/
    cp    "#{ext_ruby}/lib/lib#{ruby_so}.dylib", "dist"
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
    cp "platform/mac/Info.plist", "#{APPNAME}.app/Contents/"
    cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
    # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
    `echo -n 'APPL????' > #{APPNAME}.app/Contents/PkgInfo`
    # mv "#{APPNAME}.app/Contents/Resources/hacketyhack", "#{APPNAME}.app/Contents/MacOS"
    # cp YourAppMacIcons.icns AnotherResource.txt YourApp.app/Contents/Resources/
  when /win32/
    cp "platform/msw/shoes.exe.manifest", "dist"
  end
end

# use the platform Ruby claims
case PLATFORM
when /win32/
  OBJ = FileList["{bin,shoes}/*.{c,rc}"].map do |x|
    x.gsub(/\.c$/, '.obj').gsub(/\.rc$/, '.res')
  end

  # MSVC build environment
  MSVC_LIBS = %[msvcrt-ruby18.lib pango-1.0.lib pangocairo-1.0.lib gobject-2.0.lib glib-2.0.lib cairo.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib advapi32.lib oleacc.lib]

  MSVC_CFLAGS = %q[/MD /DWIN32 /DSHOES_WIN32
    /Iexternal\cairo\include\cairo
    /Iexternal\pango\include\pango-1.0
    /Iexternal\pango\include\glib-2.0
    /Iexternal\pango\lib\glib-2.0\include
    /Iexternal\ruby\lib\ruby\1.8\i386-mswin32
    /I.
    /O2 /GR /EHsc
  ].gsub(/\n\s*/, ' ')

  if ENV['DEBUG']
    MSVC_CFLAGS << " /DDEBUG"
  end

  # MSVC build tasks
  task :build_os => [:buildenv_win32, "dist/#{NAME}.exe"]

  task :buildenv_win32 do
    vcvars32_bat = File.join(env('MSVC'), "vcvars32.bat")
    unless File.exists?(vcvars32_bat)
      vcvars32_bat = File.join(env('MSVC'), "bin/vcvars32.bat")
    end
    `"#{vcvars32_bat}" x86 && set`.each do |line|
      if line =~ /(\w+)=(.+)/
        ENV[$1] = $2
      end
    end
    mkdir_p "dist"
  end

  task "dist/#{NAME}.exe" => OBJ do |t|
    rm_f t.name
    sh "link /NOLOGO /OUT:#{t.name} /LIBPATH:#{ext_ruby}/lib " +
      "/LIBPATH:external/cairo/lib " +
      "/LIBPATH:external/pango/lib " +
      "/SUBSYSTEM:WINDOWS #{OBJ.join(' ')} #{MSVC_LIBS}"
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
    mv "dist/installer/#{SHORTNAME}-#{VERS}.exe", "pkg"
  end
else
  require 'rbconfig'

  CC = "gcc"
  OBJ = FileList["{bin,shoes}/*.c"].map do |x|
    x.gsub(/\.\w+$/, '.o')
  end

  # Linux build environment
  CAIRO_CFLAGS = ENV['CAIRO_CFLAGS'] || `pkg-config --cflags cairo`.strip
  CAIRO_LIB = ENV['CAIRO_LIB'] ? "-L#{ENV['CAIRO_LIB']}" : `pkg-config --libs cairo`.strip
  PANGO_CFLAGS = ENV['PANGO_CFLAGS'] || `pkg-config --cflags pango`.strip
  PANGO_LIB = ENV['PANGO_LIB'] ? "-L#{ENV['PANGO_LIB']}" : `pkg-config --libs pango`.strip

  LINUX_CFLAGS = %[-I/usr/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} -I#{Config::CONFIG['archdir']}]
  LINUX_LIB_NAMES = %W[#{ruby_so} cairo pangocairo-1.0]
  if ENV['DEBUG']
    LINUX_CFLAGS << " -DDEBUG"
  end

  LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')
  case PLATFORM when /darwin/
    LINUX_CFLAGS << " -DSHOES_QUARTZ -g -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings"
    LINUX_LIBS << " -framework Carbon"
  else
    LINUX_CFLAGS << " -DSHOES_GTK #{`pkg-config --cflags gtk+-2.0`.strip}"
    LINUX_LIBS << " #{`pkg-config --libs gtk+-2.0`.strip}"
  end

  task :build_os => [:buildenv_linux, "dist/#{NAME}"]

  task :buildenv_linux do
    rm_rf "dist"
    mkdir_p "dist"
  end

  task "dist/#{NAME}" => OBJ do |t|
    bin = "#{t.name}-bin"
    rm_f t.name
    rm_f bin
    sh "#{CC} -L#{Config::CONFIG['libdir']} #{CAIRO_LIB} #{PANGO_LIB} -o #{bin} #{OBJ.join(' ')} #{LINUX_LIBS}"
    case PLATFORM when /darwin/
      mv bin, t.name
    else
      sh "echo 'LD_LIBRARY_PATH=. ./#{File.basename(bin)} $@' > #{t.name}"
      chmod 0755, t.name
    end
  end

  rule ".o" => ".mm" do |t|
    sh "#{CC} -c -o#{t.name} -fpascal-strings -Wundef -Wno-ctor-dtor-privacy -fno-strict-aliasing -fno-common #{t.source}"
  end

  rule ".o" => ".c" do |t|
    sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
  end
end
