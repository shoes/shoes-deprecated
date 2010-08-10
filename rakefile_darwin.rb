require 'make/rakefile_common'
require 'make/darwin/env'
require 'make/darwin/tasks'

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  MakeDarwin.common_build
  MakeDarwin.copy_deps_to_dist
  MakeDarwin.copy_files_to_dist
  MakeDarwin.setup_system_resources
end

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] do |t|
  MakeDarwin.make_app t.name
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  MakeDarwin.make_so t.name
end

rule ".o" => ".m" do |t|
  MakeDarwin.cc t
end

rule ".o" => ".c" do |t|
  MakeDarwin.cc t
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end

task :stub do
  MakeDarwin.make_stub
end

task :installer do
  MakeDarwin.make_installer
end
