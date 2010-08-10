require 'make/rakefile_common'

# for Mac
def copy_ext xdir, libdir
  Dir.chdir(xdir) do
    `ruby extconf.rb; make`
  end
  copy_files "#{xdir}/*.bundle", libdir
end

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  common_build
  MakeDarwin.copy_deps_to_dist
  MakeDarwin.copy_files_to_dist
  MakeDarwin.setup_system_resources
end

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
