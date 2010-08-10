require 'make/rakefile_common'
require 'make/linux/env'
require 'make/linux/tasks'

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  common_build
  MakeLinux.copy_deps_to_dist
  MakeLinux.copy_files_to_dist
  MakeLinux.setup_system_resources
end

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] do |t|
  MakeLinux.make_app t.name
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  MakeLinux.make_so t.name
end

rule ".o" => ".m" do |t|
  MakeLinux.cc t
end

rule ".o" => ".c" do |t|
  MakeLinux.cc t
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end

task :installer do
  MakeLinux.make_installer
end
