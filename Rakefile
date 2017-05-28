require 'rubygems'
require 'rake'
require 'rake/clean'
# require_relative 'platform/skel'
require 'fileutils'
require 'find'
require 'yaml'
require 'rbconfig'
include FileUtils

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
      APP['REVISION'] = '9' # make it up
    end
end


NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
APPNAME = APP['name'] # OSX needs this
SONAME = 'shoes'
APPARGS = APP['run']

RUBY_SO = RbConfig::CONFIG['RUBY_SO_NAME']
RUBY_V = RbConfig::CONFIG['ruby_version']
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']

# default exts, gems & locations to build and include - replace with custom.yaml
APP['GEMLOC'] = File.expand_path('req')
APP['EXTLOC'] = File.expand_path('req')
APP['EXTLIST'] = ['chipmunk']
APP['GEMLIST'] = ['sqlite3']

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
#CLEAN.include ["req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
CLEAN.include ["#{TGT_DIR}/libshoes.dll", "#{TGT_DIR}/*shoes.exe", 
    "#{TGT_DIR}/libshoes.so","#{TGT_DIR}/shoes", "#{TGT_DIR}/shoes-bin",
    "shoes/**/*.o", "shoes/**/*.lib"]
CLOBBER.include ["#{TGT_DIR}", "zzsetup.done", "crosscompile", "shoes/**/*.o",
    "shoes/**/*.lib"]

# for Host building for Host:
case RUBY_PLATFORM
when /mingw/
  if CROSS
    require File.expand_path("make/win32/#{TGT_ARCH}/env")
    require File.expand_path("make/win32/#{TGT_ARCH}/tasks")
    require File.expand_path("make/win32/#{TGT_ARCH}/stubs")
    require File.expand_path("make/gems")
  else
    require File.expand_path('make/win32/loose/env.rb')
    require File.expand_path('make/win32/loose/tasks.rb')
    puts "PLEASE SELECT a build environment from the win32 options "
    puts"   shown from a a `rake -T` "
  end
  Builder = MakeMinGW
  NAMESPACE = :win32

when /darwin/
  if CROSS
    # Building tight shoes on OSX for OSX
    require File.expand_path("make/darwin/#{TGT_ARCH}/env")
    require File.expand_path("make/darwin/#{TGT_ARCH}/tasks")
    require File.expand_path("make/darwin/#{TGT_ARCH}/stubs")
    require File.expand_path("make/gems")
  else
    # build Loose Shoes on OSX for OSX
    puts "OSX: please select a target - see rake -T"
    puts "This Shoes may not be portable to other OSX systems"
    require File.expand_path('make/darwin/loose/env')
    require File.expand_path('make/darwin/loose/tasks')
  end
  Builder = MakeDarwin
  NAMESPACE = :osx
  
when /linux/
  if CROSS
    # This will be a Tight Shoes setup
    case TGT_ARCH
    when /x86_64-linux/
      require File.expand_path('make/linux/x86_64-linux/env')
      require File.expand_path('make/linux/x86_64-linux/tasks')
      require File.expand_path('make/subsys')
      require File.expand_path("make/gems")
      require File.expand_path('make/linux/x86_64-linux/setup')
    when /i686-linux/
      require File.expand_path('make/linux/i686-linux/env')
      require File.expand_path('make/linux/i686-linux/tasks')
      require File.expand_path("make/gems")
      require File.expand_path('make/linux/i686-linux/setup')
    when /pi2/
      require File.expand_path('make/linux/pi2/env')
      require File.expand_path('make/linux/pi2/tasks')
      require File.expand_path('make/linux/pi2/setup')
      require File.expand_path("make/gems")
    when /xarmv6hf/
      require File.expand_path('make/linux/xarm6hf/env')
      require File.expand_path('make/linux/xarm6hf/tasks')
      require File.expand_path('make/gems')
   when /xwin7/
      require File.expand_path('make/linux/xwin7/env')
      require File.expand_path('make/subsys')
      require File.expand_path('make/linux/xwin7/tasks')
      require File.expand_path('make/linux/xwin7/stubs')
      require File.expand_path('make/linux/xwin7/setup')
      require File.expand_path('make/linux/xwin7/packdeps')
      require File.expand_path('make/gems')
   when /xmsys2/
      require File.expand_path('make/linux/xmsys2/env')
      require File.expand_path('make/linux/xmsys2/tasks')
      require File.expand_path('make/linux/xmsys2/stubs')
      require File.expand_path('make/linux/xmsys2/packdeps')
      require File.expand_path('make/linux/xmsys2/setup')
      require File.expand_path('make/gems')
   else
      puts "Unknown builder for #{TGT_ARCH}, removing setting"
      rm_rf "crosscompile" if File.exists? "crosscompile"
    end
  else
     # This is Loose Shoes setup
     #TGT_DIR = "dist"
     require File.expand_path('make/subsys')
     require File.expand_path('make/linux/loose/env')
     require File.expand_path('make/linux/loose/tasks')
     require File.expand_path('make/linux/loose/setup')
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

task :build_os => ["#{TGT_DIR}/#{NAME}"]

task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << "#ifndef SHOES_VERSION_H\n"
    f << "#define SHOES_VERSION_H\n\n"
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
      f << "#define SHOES_STYLE \"TIGHT_SHOES\"\n\n"
    else
      f << "#define SHOES_STYLE \"LOOSE_SHOES\"\n\n"
    end
    f << "extern VALUE cTypes;\n"
    f << "\nvoid shoes_version_init();\n\n"
    f << "#endif\n"
  end
end

# TODO: Left for historical reasons (aka OSX)
task "#{TGT_DIR}/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION}) [#{SHOES_RUBY_ARCH} Ruby#{RUBY_V}]}
    %w[DEBUG].each { |x| f << " +#{x.downcase}" if ENV[x] }
    f << "\n"
  end
end

#TODO: should the following be a task or file? 
file "shoes/types/types.h" do |t|
   puts "Processing #{t.name}..."
   
   rm_rf "shoes/types/types.h" if File.exists? "shoes/types/types.h"
   
   headers =  Dir["shoes/types/*.h"] - ["shoes/types/types.h"]
   content = headers.collect { |file|
      File.read(file).scan(/shoes_[[:alnum:]_]+_init\(\);/)
   }.flatten

   File.open(t.name, 'w') do |f|
      headers.sort.each { |header|
         f << "#include \"#{header}\"\n"
      }
      f << "\n#define SHOES_TYPES_INIT \\\n#{content.sort.collect { |n| "\t#{n}" }.join(" \\\n") }\n"
   end
end

def create_version_file file_path
  File.open(file_path, 'w') do |f|
    f << "shoes #{APP['NAME'].downcase} #{APP['VERSION']} r(#{APP['REVISION']}) #{APP['PLATFORM']} #{APP['DATE']}"
    f << "\n"
  end
end

# TODO: called from osx(s) copy_files_to_dist in task.rb
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

# --------------------------
# tasks depending on Builder = MakeLinux|MakeDarwin|MakeMinGW

desc "Build using your OS setup"
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

# ---------  newer build - used by linux, so far -------

file  "zzsetup.done" do
  Builder.static_setup SOLOCS
  Builder.copy_gems #used to be common_build
  Builder.setup_system_resources
  touch "zzsetup.done"
end

task :static_setup do
  $stderr.puts "rake calls :static setup" 
  #Builder.static_setup SOLOCS
end

SubDirs = ["shoes/base.lib", "shoes/http/download.lib", "shoes/plot/plot.lib",
    "shoes/console/console.lib", "shoes/types/widgets.lib", "shoes/native/native.lib"]
    
# Windows does't use console - don't try to build it.
case TGT_DIR
  when 'win7', 'xwin7', 'msys2', 'xmsys2'
    SubDirs.delete("shoes/console/console.lib")
end
file "#{TGT_DIR}/libshoes.so" => ["zzsetup.done", "shoes/types/types.h"] + SubDirs do
  Builder.new_so "#{TGT_DIR}/libshoes.so"
end

#task :new_build => ["zzsetup.done", "shoes/types/types.h"] + SubDirs  do
task :new_build => "#{TGT_DIR}/libshoes.so"  do
  # We can link shoes here - this can be done via a Builder call or

  Builder.new_link "#{TGT_DIR}/shoes"
  $stderr.puts "new build: called for #{TGT_DIR}"
end

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

task "#{TGT_DIR}/#{NAME}" => ["#{TGT_DIR}/lib#{SONAME}.#{DLEXT}", "shoes/main.o"] + ADD_DLL + ["#{NAMESPACE}:make_app"]

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
    desc "Setup to build Shoes for 10.10+"
    task :yosemite do
      sh "echo 'TGT_ARCH=yosemite' >crosscompile"
    end

    desc "Setup to build Shoes for 10.9+ from 10.10+"
    task :xmavericks do
      sh "echo 'TGT_ARCH=xmavericks' >crosscompile"
    end

    desc "Setup to build Shoes for 10.9+ from 10.9"
    task :mavericks do
      sh "echo 'TGT_ARCH=mavericks' >crosscompile"
    end

    #desc "Setup to build for 10.6+ from 10.6"
    #task :snow do
    #  sh "echo 'TGT_ARCH=snow' >crosscompile"
    #end

    #desc "Downshift Build 10.6 from 10.9"
    #task "xsnow" do
    #  sh "echo 'TGT_ARCH=xsnow' >crosscompile"
    #end

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
    desc "Winodws build with devkit"
    task :win7 do
      sh "echo TGT_ARCH=win7 >crosscompile"
    end

    desc "Windows build with msys2"
    task :msys2 do
      sh "echo TGT_ARCH=msys2 >crosscompile"
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
    desc "Cross compile to arm6hf - advanced users"
    task :xarm6hf do
      sh "echo 'TGT_ARCH=xarmv6hf' >crosscompile"
    end
    
    desc "Native pi2 build"
    task :pi2 do
      sh "echo 'TGT_ARCH=pi2' >crosscompile"
    end
    
    desc "Cross compile for msys2 deps (mingw)"
    task :xmsys2 do
      puts "Cross compile newer deps (mingw)"
      sh "echo 'TGT_ARCH=xmsys2' >crosscompile"
    end

    desc "Cross compile to MingW32 (Gtk, 32)"
    task :xwin7 do
      puts "Cross compile for Windows MingW32"
      sh "echo 'TGT_ARCH=xwin7' >crosscompile"
    end

    desc "chroot build for i686 (32bit linux)"
    task :i686_linux do
      puts "Cross complile for i686-linux"
      sh "echo 'TGT_ARCH=i686-linux' >crosscompile"
    end

    desc "chroot build for x86_64 (64bit linux)"
    task :x86_64_linux do
      puts "Cross complile for x86_64-linux"
      sh "echo 'TGT_ARCH=x86_64-linux' >crosscompile"
    end

  end
  
  #task :build => [:old_build]
  
  task :static_setup do 
    Builder.static_setup SOLOCS
  end
  
  task :build => [:new_build]

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
