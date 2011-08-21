require 'rubygems'
require 'rake'
require 'rake/clean'
require_relative 'platform/skel'
require 'fileutils'
require 'find'
require 'yaml'
include FileUtils

APP = YAML.load_file(File.join(ENV['APP'] || ".", "app.yaml"))
APPNAME = APP['name']
RELEASE_ID, RELEASE_NAME = APP['version'], APP['release']
NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
SONAME = 'shoes'

require 'cucumber'
require 'cucumber/rake/task'

Cucumber::Rake::Task.new(:features) do |t|
  t.cucumber_opts = "--format pretty"
end

require 'bundler'
Bundler::GemHelper.install_tasks

GIT = ENV['GIT'] || "git"
REVISION = (`#{GIT} rev-list HEAD`.split.length + 1).to_s
VERS = ENV['VERSION'] || "0.r#{REVISION}"
PKG = "#{NAME}-#{VERS}"
APPARGS = APP['run']
FLAGS = %w[DEBUG]

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "dist"]

RUBY_SO = Config::CONFIG['RUBY_SO_NAME']
RUBY_V = Config::CONFIG['ruby_version']

if ENV['APP']
  %w[dmg icons].each do |subk|
    APP[subk].keys.each do |name|
      APP[subk][name] = File.join(ENV['APP'], APP[subk][name])
    end
  end
end

if File.exists? ".git/refs/tags/#{RELEASE_ID}/#{RELEASE_NAME}"
  abort "** Rename this release (and add to lib/shoes.rb) #{RELEASE_NAME} has already been tagged."
end

# Same effect as sourcing a shell script before running rake. It's necessary to
# set these values before the make/{platform}/env.rb files are loaded.
def osx_bootstrap_env
  ENV['DYLD_LIBRARY_PATH'] = '/usr/local/Cellar/cairo/1.10.2/lib:/usr/local/Cellar/cairo/1.10.2/include/cairo'
  ENV['LD_LIBRARY_PATH'] = '/usr/local/Cellar/cairo/1.10.2/lib:/usr/local/Cellar/cairo/1.10.2/include/cairo'
  ENV['CAIRO_CFLAGS'] = '-I/usr/local/Cellar/cairo/1.10.2/include/cairo'
  ENV['SHOES_DEPS_PATH'] = '/usr/local'
end

case RUBY_PLATFORM
when /mingw/
  require File.expand_path('rakefile_mingw')
  Builder = MakeMinGW
when /darwin/
  osx_bootstrap_env
  require File.expand_path('rakefile_darwin')
  Builder = MakeDarwin
when /linux/
  require File.expand_path('rakefile_linux')
  Builder = MakeLinux
else
  puts "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
end

# --------------------------
# common platform tasks

desc "Same as `rake build'"
task :default => [:build]

desc "Does a full compile, with installer"
task :package => [:build, :installer]

task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << %{#define SHOES_RELEASE_ID #{RELEASE_ID}\n#define SHOES_RELEASE_NAME "#{RELEASE_NAME}"\n#define SHOES_REVISION #{REVISION}\n#define SHOES_BUILD_DATE #{Time.now.strftime("%Y%m%d")}\n#define SHOES_PLATFORM "#{RUBY_PLATFORM}"\n}
  end
end

task "dist/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION}) [#{RUBY_PLATFORM} Ruby#{RUBY_V}]}
    %w[DEBUG].each { |x| f << " +#{x.downcase}" if ENV[x] }
    f << "\n"
  end
end

# shoes is small, if any include changes, go ahead and build from scratch.
SRC.zip(OBJ).each do |c, o|
  file o => [c] + Dir["shoes/*.h"]
end

# --------------------------
# tasks depending on Builder = MakeLinux|MakeDarwin|MakeMinGW

desc "Does a full compile, for the OS you're running on"
task :build => [:build_os, "dist/VERSION.txt"] do
  Builder.common_build
  Builder.copy_deps_to_dist
  Builder.copy_files_to_dist
  Builder.setup_system_resources
end

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] + ADD_DLL do |t|
  Builder.make_app t.name
end

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h'] + OBJ do |t|
  Builder.make_so t.name
end

rule ".o" => ".m" do |t|
  Builder.cc t
end

rule ".o" => ".c" do |t|
  Builder.cc t
end

task :installer do
  Builder.make_installer
end

# Like puts, but only if we've --trace'd
def vputs(str)
  puts str if Rake.application.options.trace
end

task :osx_deps do
  homebrew_install "cairo"
  homebrew_install "pango"
  homebrew_install "libjpeg"
  homebrew_install "giflib"
  homebrew_install "libiconv"
  homebrew_install "portaudio"
  homebrew_install "gettext"
  homebrew_install "cairo"
  homebrew_install "gettext"
end

def homebrew_install package
  sh %{brew list #{package}} do |ok, res|
    if ok
      sh "brew install #{package}"
    else
      vputs "#{package} already exists, continuing"
    end
  end
end

