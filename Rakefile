require 'rubygems'
require 'rake'
require 'rake/clean'
# require_relative 'platform/skel'
require 'fileutils'
require 'find'
require 'yaml'
require 'rbconfig'
include FileUtils

# Like puts, but only if we've --trace'd
def vputs(str)
  puts str if Rake.application.options.trace
end

APP = YAML.load_file(File.join(ENV['APP'] || ".", "app.yaml"))
# APP['version'] = APP['major'] # for historical reasons 
# populate APP[] with uppercase names and string values
APP['VERSION'] = "#{APP['major']}.#{APP['minor']}.#{APP['tiny']}"
APP['MAJOR'] = APP['major'].to_s
APP['MINOR'] = APP['minor'].to_s
APP['TINY'] = APP['tiny'].to_s
APP['NAME'] = APP['release']
APP['DATE'] = Time.now.to_s
APP['PLATFORM'] = RbConfig::CONFIG['arch'] # not correct in cross compile
case APP['revision']
  when 'git'
    GIT = ENV['GIT'] || "git"
    APP['REVISION'] = (`#{GIT} rev-list HEAD`.split.length).to_s
  when 'file'
    File.open('VERSION.txt', 'r') do |f|
      ln = f.read
      rev = ln[/r\(\d+\)/]
      APP['REVISION'] = "#{rev[/\d+/].to_i + 1}"
    end
  else
    if APP['revision'].kind_of? Fixnum
      APP['REVISION'] = APP['revision'].to_s
    else
      APP['REVISION'] = '0009' # make it up 
    end
end

# delete from here
VERS = "#{APP['MAJOR']}.#{APP['MINOR']}"
REVISION = VERS
RELEASE_ID, RELEASE_NAME = APP['major'], APP['release']
if RUBY_PLATFORM =~ /darwin/
  #APPNAME = "#{APP['name']}-#{RELEASE_NAME}"
  APPNAME = APP['name']
else
  APPNAME = APP['name']
end
NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
SONAME = 'shoes'


TINYVER = APP['tiny']
PKG = "#{NAME}-#{VERS}"
#MENU_NAME = "#{APPNAME} #{VERS}#{TINYVER}" 
APPARGS = APP['run']
FLAGS = %w[DEBUG]
# to here

RUBY_SO = RbConfig::CONFIG['RUBY_SO_NAME']
RUBY_V = RbConfig::CONFIG['ruby_version']
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']

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
  ENV['CAIRO_CFLAGS'] = '-I/usr/local/Cellar/cairo/1.12.16_1/include/cairo'
  ENV['GLIB_CFLAGS'] = '-I/usr/local/Cellar/glib/2.40.0/include/glib-2.0'
  #ENV['PKG_CONFIG_PATH'] = '/opt/X11/lib/pkgconfig' # check spelling X11
  ENV['SHOES_DEPS_PATH'] = '/usr/local'
end

if File.exists? "crosscompile"
  CROSS = true
  File.open('crosscompile','r') do |f|
    str = f.readline
    TGT_ARCH = str.split('=')[1].strip
    if ENV['NFS_ALTP']
      TGT_DIR = ENV['NFS_ALTP']+TGT_ARCH
       mkdir_p "#{TGT_DIR}"
    else
      TGT_DIR = TGT_ARCH
    end
  end
else
  CROSS = false
  # is the build directory outside the project dir like
  # Cecil's weird NFS setup for his mac
  if ENV['NFS_ALTP']
    TGT_DIR = ENV['NFS_ALTP']+'dist'
    mkdir_p "#{TGT_DIR}"
  else
    TGT_DIR = 'dist'
  end
end

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
#CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
CLEAN.include ["req/**/#{BIN}", "#{TGT_DIR}", "*.app"]

# for Host building for Host:
case RUBY_PLATFORM
when /mingw/
  if CROSS
    require File.expand_path("make/tightmingw/env")
    require File.expand_path("make/tightmingw/tasks")
    require File.expand_path("make/tightmingw/stubs")
  else
    require File.expand_path('rakefile_mingw')
  end
  
  Builder = MakeMinGW
  NAMESPACE = :win32
  
when /darwin/
  osx_bootstrap_env

  if CROSS
    # Building tight shoes on OSX for OSX
    require File.expand_path("make/#{TGT_ARCH}/env")
    #require_relative "make/#{TGT_ARCH}/homebrew"
    require File.expand_path("make/#{TGT_ARCH}/tasks")
    require File.expand_path("make/#{TGT_ARCH}/stubs")
  else
    # build Loose Shoes on OSX for OSX
    puts "Loose Shoes OSX"
    require File.expand_path('make/darwin/env')
    require File.expand_path('make/darwin/tasks')
  end
  Builder = MakeDarwin
  NAMESPACE = :osx
when /linux/
  if CROSS
    # This will be a Tight Shoes setup
    case TGT_ARCH
    when /x86_64-linux/ 
      require File.expand_path('make/x86_64-linux/env')
      require File.expand_path('make/x86_64-linux/tasks')
    when /i386-gnu-linux/
      require File.expand_path('make/xi386-gnu-linux/env')
      require File.expand_path('make/xi386-gnu-linux/tasks')
    when /xarmv6-pi/
      require File.expand_path('make/xarmv6-pi/env')
      require File.expand_path('make/xarmv6-pi/tasks')
    when /xmingw32/
      require File.expand_path('make/xmingw32/env')
      require File.expand_path('make/xmingw32/tasks')
      require File.expand_path('make/xmingw32/stubs')
    when /xmsw32/
      require File.expand_path('make/xmsw32/env')
      require File.expand_path('make/xmsw32/tasks')
    #when /xsnow/
    #  require File.expand_path('make/xsnow/env')
    #  require File.expand_path('make/xsnow/tasks')
   else
      puts "Unknown builder for #{TGT_ARCH}, removing setting"
      rm_rf "crosscompile" if File.exists? "crosscompile"
    end
  else
     # This is Loose Shoes setup
     #TGT_DIR = "dist"
     require File.expand_path('make/linux/env')
     require File.expand_path('make/linux/tasks')
  end
  Builder = MakeLinux
  NAMESPACE = :linux
else
  puts "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
end

# --------------------------
# common platform tasks

desc "Same as `rake build'"
task :default => [:build]

desc "Package Shoes for distribution"
task :package => [:version, :installer]

task :build_os => [:build_skel, "#{TGT_DIR}/#{NAME}"]

task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << "// compatatibily pre 3.2.22\n"
    f << "#define SHOES_RELEASE_ID #{APP['MAJOR']}\n"
    f << "#define SHOES_REVISION #{APP['REVISION']}\n"
    f << "#define SHOES_RELEASE_NAME \"#{APP['NAME']}\"\n"
    f << "#define SHOES_BUILD_DATE \"#{APP['DATE']}\"\n"
    f << "#define SHOES_PLATFORM \"#{SHOES_RUBY_ARCH}\"\n"
    f << "// post 3.2.22\n"
    f << "#define SHOES_VERSION_NUMBER \"#{APP['VERSION']}\"\n"
    f << "#define SHOES_VERSION_MAJOR #{APP['MAJOR']}\n"
    f << "#define SHOES_VERSION_MINOR #{APP['MINOR']}\n"
    f << "#define SHOES_VERSION_TINY #{APP['TINY']}\n"
    f << "#define SHOES_VERSION_NAME \"#{APP['NAME']}\"\n"
    f << "#define SHOES_VERSION_REVISION #{APP['REVISION']}\n"
    f << "#define SHOES_VERSION_DATE \"#{APP['DATE']}\"\n"
    f << "#define SHOES_VERSION_PLATFORM \"#{APP['PLATFORM']}\"\n"
    if CROSS
      f << '#define SHOES_STYLE "TIGHT_SHOES"'
    else
      f << '#define SHOES_STYLE "LOOSE_SHOES"'
    end
  end
end

# FIXME: Left for historical reasons (aka OSX)
task "#{TGT_DIR}/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION}) [#{SHOES_RUBY_ARCH} Ruby#{RUBY_V}]}
    %w[DEBUG].each { |x| f << " +#{x.downcase}" if ENV[x] }
    f << "\n"
  end
end

def create_version_file file_path
  File.open(file_path, 'w') do |f|
    f << "shoes #{APP['NAME'].downcase} #{APP['VERSION']} r(#{APP['REVISION']}) #{APP['PLATFORM']} #{APP['DATE']}"
    f << "\n"
  end
end

# FIXME: called from osx(s) copy_files_to_dist in task.rb 
def osx_version_txt t
  create_version_file t
end

desc "create VERSION.txt"
task :version do
 create_version_file 'VERSION.txt'
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

task :pre_build do
  Builder.pre_build
end

# first refactor ; build calls platform namespaced build;
# for now, each of those calls the old build method.
task :old_build => [:pre_build, :build_os] do
  Builder.common_build
  Builder.copy_files_to_dist
  Builder.copy_deps_to_dist
  Builder.setup_system_resources
end

#desc "Build Shoes gems"
#task :gems do
#  Builder.gems_build
#end

desc "Install Shoes in your ~/.shoes Directory"
task  :install do
  if CROSS 
     puts "Sorry. You can't do an install of your source built Shoes"
     puts "when crosscompiling is setup."
  else
    create_version_file 'VERSION.txt'
    Builder.copy_files_to_dist
    Builder.make_userinstall
  end
end


directory "#{TGT_DIR}"	# was 'dist'

task "#{TGT_DIR}/#{NAME}" => ["#{TGT_DIR}/lib#{SONAME}.#{DLEXT}", "bin/main.o"] + ADD_DLL + ["#{NAMESPACE}:make_app"]

task "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}" => ['shoes/version.h', "#{TGT_DIR}"] + OBJ + ["#{NAMESPACE}:make_so"]

def cc(t)
  sh "#{CC} -I. -c -o #{t.name} #{LINUX_CFLAGS} #{t.source}"
end

rule ".o" => ".m" do |t|
  cc t
end

rule ".o" => ".c" do |t|
  cc t
end

task :installer => ["#{NAMESPACE}:installer"]

def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
  File.open(after, 'w') do |a|
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(reg) do
          if reg2.include? '\1'
            reg2.gsub(%r!\\1!, Object.const_get($1))
          else
            reg2
          end
        end
      end
    end
  end
end

def copy_files glob, dir
  FileList[glob].each { |f| cp_r f, dir }
end

namespace :osx do
  namespace :setup do
    desc "Setup to build Shoes for 10.9+"
    task :mavericks do
      sh "echo 'TGT_ARCH=mavericks-x86_64' >crosscompile"
    end
    
    desc "Setup to build for 10.6+ from 10.6"
    task :snow do
      sh "echo 'TGT_ARCH=snowleopard' >crosscompile"
    end
    
    desc "Experimental Build 10.6 from 10.9"
    task "xsnow" do
      sh "echo 'TGT_ARCH=xsnow' >crosscompile"
    end
        
    desc "Setup to build Shoes just for my Mac (default)"
    task :clean do
      rm_rf "crosscompile"
    end 
  end
  
  task :build => [:old_build]

  task :make_app do
    Builder.make_app "#{TGT_DIR}/#{NAME}"
  end

  task :make_so do
    Builder.make_so  "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}"
  end

  task :installer do
    Builder.make_installer
  end

end


namespace :win32 do
  
  namespace :setup do
    desc "Build for distribution (Tight)"
    task :tight do
      puts "Windows tight build"
      sh "echo TGT_ARCH=tightmingw >crosscompile"      
    end
    
    desc "Loose setup"
    task :clean do
      puts "restored to Loose Shoes build"
      rm_rf "crosscompile" if File.exists? "crosscompile"
    end
    
  end
  
  task :build => [:old_build]

  task :make_app do
    Builder.make_app "#{TGT_DIR}/#{NAME}"
  end

  task :make_so do
    Builder.make_so  "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}"
  end

  task :installer do
    Builder.make_installer
  end
end

# get here when rake decides it's running on a linux system
namespace :linux do

  namespace :setup do
    desc "Cross compile to Raspberry pi"
    task :rpi do
      puts "Cross compile to Raspberry setup"
      sh "echo 'TGT_ARCH=xarmv6-pi' >crosscompile"
    end

#    desc "Cross compile to Windows Native GUI"
#    task :msw32 do
#      puts "Cross compile for WIN32"
#      sh "echo 'TGT_ARCH=xmsw32' >crosscompile"
#    end

    desc "Cross compile to MingW32 (Gtk)"
    task :mingw32 do
      puts "Cross compile for Windows MingW32"
      sh "echo 'TGT_ARCH=xmingw32' >crosscompile"
    end

    desc "Cross compile to i686 32bit linux"
    task :x32 do
      puts "Cross complile to i386-gnu-linux setup"
      sh "echo 'TGT_ARCH=i386-gnu-linux' >crosscompile"
    end
    
    desc "Cross compile to x86_64-linux"
    task :x86 do
      puts "Cross complile to x86_64-linux setup"
      sh "echo 'TGT_ARCH=x86_64-linux' >crosscompile"
    end
    
    desc "Remove cross compile setup"
    task :clean do
      puts "restored to native build"
      rm_rf "crosscompile" if File.exists? "crosscompile"
    end
  end
  
  task :build => [:old_build]

  task :make_app do
    Builder.make_app "#{TGT_DIR}/#{NAME}"
  end

  task :make_so do
    Builder.make_so  "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}"
  end

  task :installer do
    Builder.make_installer
  end
  
end

# Note the following works: Not pretty but it works
#task "#{TGT_DIR}/libshoes.so" do
#   Builder.make_so  "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}"
#end
