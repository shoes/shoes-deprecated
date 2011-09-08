require 'rubygems'
require 'rake'
require 'rake/clean'
# require_relative 'platform/skel'
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
  NAMESPACE = :win32
when /darwin/
  osx_bootstrap_env
  require File.expand_path('make/darwin/env')
  require File.expand_path('make/darwin/tasks')

  task :stub do
    MakeDarwin.make_stub
  end
  Builder = MakeDarwin
  NAMESPACE = :osx
when /linux/
  require File.expand_path('rakefile_linux')
  Builder = MakeLinux
  NAMESPACE = :linux
else
  puts "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
end

# --------------------------
# common platform tasks

desc "Same as `rake build'"
task :default => [:build]

desc "Does a full compile, with installer"
task :package => [:build, :installer]

task :build_os => [:build_skel, "dist/#{NAME}"]

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

# ------
# skel

def skel_replace(line)
  line.gsub! /\s+%DEFAULTS%/ do
    if APPARGS
      args = APPARGS.split(/\s+/)
      %{
        char *default_argv[] = {argv[0], #{args.inspect[1..-2]}};
        argv = default_argv;
        argc = #{args.length + 1};
      }
    end
  end
  line
end

# preprocess .skel
task :build_skel do |t|
  Dir["bin/*.skel"].each do |src|
    name = src.gsub(/\.skel$/, '.c')
    File.open(src) do |skel|
      File.open(name, 'w') do |c|
        skel.each_line do |line|
          c << skel_replace(line)
        end
      end
    end
  end
end

# --------------------------
# tasks depending on Builder = MakeLinux|MakeDarwin|MakeMinGW

desc "Does a full compile, for the OS you're running on"
task :build => ["#{NAMESPACE}:build"]

# first refactor ; build calls platform namespaced build;
# for now, each of those calls the old build method.
task :old_build => [:build_os, "dist/VERSION.txt"] do
  Builder.common_build
  Builder.copy_deps_to_dist
  Builder.copy_files_to_dist
  Builder.setup_system_resources
end

directory 'dist'

task "dist/#{NAME}" => ["dist/lib#{SONAME}.#{DLEXT}", "bin/main.o"] + ADD_DLL + ["#{NAMESPACE}:make_app"]

task "dist/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h', 'dist'] + OBJ + ["#{NAMESPACE}:make_so"]

rule ".o" => ".m" do |t|
  Builder.cc t
end

rule ".o" => ".c" do |t|
  Builder.cc t
end

task :installer => ["#{NAMESPACE}:installer"] do
  #Builder.make_installer
end

# Like puts, but only if we've --trace'd
def vputs(str)
  puts str if Rake.application.options.trace
end

namespace :osx do
  namespace :deps do
    desc "Installs OS X dependencies"
    task :install => :bootstrap do
      homebrew_install "cairo"
      sh "brew link cairo" unless File.exist?("/usr/local/lib/libcairo.2.dylib")
      homebrew_install "pango"
      homebrew_install "jpeg"
      homebrew_install "giflib"
      homebrew_install "portaudio"
      homebrew_install "gettext"
      homebrew_install "cairo"
      homebrew_install "gettext"
    end
    
    task :bootstrap do
      # For now, pull in this patched glib formula
      cd `brew --prefix`.chomp do
        unless `git remote`.split.include?('shoes')
          sh "git remote add shoes git://github.com/wasnotrice/homebrew.git"
        end
        sh "git fetch shoes"
        sh "git merge shoes/shoes"
      end
    end
  end

  task :build => [:build_skel, "dist/#{NAME}", "dist/VERSION.txt"] do
    Builder.common_build
    Builder.copy_deps_to_dist
    Builder.copy_files_to_dist
    Builder.setup_system_resources
  end

  task :make_app do
    # Builder.make_app "dist/#{NAME}"
    bin = "dist/#{NAME}-bin"
    rm_f "dist/#{NAME}"
    rm_f bin
    sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes -arch x86_64"
  end

  task :make_so do
    name = "dist/lib#{SONAME}.#{DLEXT}"
    ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
      %w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
        sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{name}"
      end
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
end

namespace :win32 do
  task :build => [:old_build]

  task :make_app do
    Builder.make_app "dist/#{NAME}"
  end

  task :make_so do
    Builder.make_so  "dist/lib#{SONAME}.#{DLEXT}"
  end

  task :installer do
    Builder.make_installer
  end
end

namespace :linux do
  task :build => [:old_build]

  task :make_app do
    Builder.make_app "dist/#{NAME}"
  end

  task :make_so do
    Builder.make_so  "dist/lib#{SONAME}.#{DLEXT}"
  end

  task :installer do
    Builder.make_installer
  end
end

def homebrew_install package
  if `brew list`.split.include?(package)
    vputs "#{package} already exists, continuing"
  else
    sh "brew install #{package}"
  end
end

