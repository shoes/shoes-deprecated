# for Linux platform
def copy_ext xdir, libdir
  Dir.chdir(xdir) do
    unless system "ruby", "extconf.rb" and system "make"
      raise "Extension build failed"
    end
  end
  copy_files "#{xdir}/*.so", libdir
end

EXT_RUBY = "deps/ruby"

unless File.exists? EXT_RUBY
  EXT_RUBY = Config::CONFIG['prefix']
end

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  common_build
  cp    "#{EXT_RUBY}/lib/lib#{RUBY_SO}.so", "dist/lib#{RUBY_SO}.so"
  ln_s  "lib#{RUBY_SO}.so", "dist/lib#{RUBY_SO}.so.#{RUBY_V[/^\d+\.\d+/]}"
  cp    "/usr/lib/libgif.so", "dist/libgif.so.4"
  ln_s  "libgif.so.4", "dist/libungif.so.4"
  cp    "/usr/lib/libjpeg.so", "dist/libjpeg.so.8"
  cp    "/usr/lib/libcurl.so", "dist/libcurl.so.4"
  cp    "/usr/lib/libportaudio.so", "dist/libportaudio.so.2"
  cp    "/usr/lib/libsqlite3.so", "dist/libsqlite3.so.0"
  
  if ENV['VIDEO']
    cp    "/usr/lib/libvlc.so", "dist"
    ln_s  "libvlc.so", "dist/libvlc.so.0"
  end
  
  sh    "strip -x dist/*.so.*"
  sh    "strip -x dist/*.so"

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
  cp APP['icons']['gtk'], "dist/static/app-icon.png"
end

# use the platform Ruby claims
require 'rbconfig'

CC = ENV['CC'] ? ENV['CC'] : "gcc"
file_list = ["shoes/*.c"] + %w{shoes/native/gtk.c shoes/http/curl.c}

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

if ENV['VIDEO']
  VLC_CFLAGS = '-I/usr/include/vlc'
  VLC_LIB = '-llibvlc'
else
  VLC_CFLAGS = VLC_LIB = ''
end

LINUX_CFLAGS = %[-Wall -I#{ENV['SHOES_DEPS_PATH'] || "/usr"}/include #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{VLC_CFLAGS} -I#{Config::CONFIG['archdir']}]
if Config::CONFIG['rubyhdrdir']
  LINUX_CFLAGS << " -I#{Config::CONFIG['rubyhdrdir']} -I#{Config::CONFIG['rubyhdrdir']}/#{RUBY_PLATFORM}"
end

LINUX_LIB_NAMES = %W[#{RUBY_SO} cairo pangocairo-1.0 ungif]

FLAGS.each do |flag|
  LINUX_CFLAGS << " -D#{flag}" if ENV[flag]
end

if ENV['DEBUG']
  LINUX_CFLAGS << " -g -O0 "
else
  LINUX_CFLAGS << " -O "
end
LINUX_CFLAGS << " -DRUBY_1_9" if RUBY_1_9

DLEXT = "so"
LINUX_CFLAGS << " -DSHOES_GTK -fPIC #{`pkg-config --cflags gtk+-2.0`.strip} #{`curl-config --cflags`.strip}"
LINUX_LDFLAGS =" #{`pkg-config --libs gtk+-2.0`.strip} #{`curl-config --libs`.strip} -fPIC -shared"
LINUX_LIB_NAMES << 'jpeg'
LINUX_LIB_NAMES << 'rt'

if ENV['VIDEO']
  if VLC_0_8
    LINUX_CFLAGS << " -DVLC_0_8"
  else
    LINUX_CFLAGS << " -I/usr/include/vlc/plugins"
  end
  LINUX_LIB_NAMES << "vlc"
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
  sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']}"
  rewrite "platform/nix/shoes.launch", t.name, %r!/shoes-bin!, "/#{NAME}-bin"
  sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{t.name}}
  chmod 0755, t.name
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
  sh "#{CC} -o #{t.name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
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

task :installer do
  mkdir_p "pkg"
  sh "makeself dist pkg/#{PKG}.run '#{APPNAME}' ./#{NAME}"
end
