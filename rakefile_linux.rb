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

task :build_os => [:buildenv_linux, :build_skel, "dist/#{NAME}"]

task :buildenv_linux do
  unless ENV['VIDEO']
    rm_rf "dist"
    mkdir_p "dist"
  end
end

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
