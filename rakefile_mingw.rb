require 'make/rakefile_common'
require 'make/mingw/env'
require 'make/mingw/tasks'

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  MakeMinGW.common_build
  MakeMinGW.copy_deps_to_dist
  MakeMinGW.copy_files_to_dist
  MakeMinGW.setup_system_resources
end

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o", "shoes/appwin32.o"] do |t|
  MakeMinGW.make_app t.name
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  MakeMinGW.make_so t.name
end

rule ".o" => ".rc" do |t|
  MakeMinGW.make_resource t
end

rule ".o" => ".m" do |t|
  MakeMinGW.cc t
end

rule ".o" => ".c" do |t|
  MakeMinGW.cc t
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end

# packaging Shoes
task :installer do
  MakeMinGW.make_installer
end
