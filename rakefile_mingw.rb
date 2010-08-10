require 'make/rakefile_common'
require 'make/mingw/env'
require 'make/mingw/tasks'

# Execute shell calls through bash if we are compiling with mingw. This breaks us
# out of the windows command shell if we are compiling from there.
def sh(*args)
  cmd = args.join(' ')
  super "bash.exe --login -i -c \"#{cmd}\""
end

def copy_ext xdir, libdir
  Dir.chdir(xdir) do
    sh 'ruby extconf.rb; make'
  end
  copy_files "#{xdir}/*.so", libdir
end

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  common_build
  MakeMinGW.copy_deps_to_dist
  MakeMinGW.copy_files_to_dist
  MakeMinGW.setup_system_resources
end

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o", "shoes/appwin32.o"] do |t|
  bin = t.name
  rm_f bin
  sh "#{CC} -Ldist -o #{bin} bin/main.o shoes/appwin32.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']} -mwindows"
  rewrite "platform/nix/shoes.launch", t.name, %r!/shoes!, "/#{NAME}"
  sh %{echo 'cd "$OLDPWD"'}
  sh %{echo 'LD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{t.name}}
  chmod 0755, t.name
  cp "platform/msw/shoes.exe.manifest", "dist/#{NAME}.exe.manifest"
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
  sh "#{CC} -o #{t.name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
end

rule ".o" => ".rc" do |t|
  sh "windres -I. #{t.source} #{t.name}"
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

# packaging Shoes
task :installer do
  def sh(*args); super; end
  mkdir_p "pkg"
  rm_rf "dist/nsis"
  cp_r  "platform/msw", "dist/nsis"
  cp APP['icons']['win32'], "dist/nsis/setup.ico"
  rewrite "dist/nsis/base.nsi", "dist/nsis/#{NAME}.nsi"
  Dir.chdir("dist/nsis") do
    sh "\"#{env('NSIS')}\\makensis.exe\" #{NAME}.nsi"
  end
  mv "dist/nsis/#{PKG}.exe", "pkg"
end
