require 'rubygems'
require 'rake'
require 'rake/clean'
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
APP['PLATFORM'] = RbConfig::CONFIG['arch'] # not correct for cross compile
case APP['revision']
  when 'git'
    gitp = ENV['GIT'] || "git"
    APP['REVISION'] = (`#{gitp} rev-list HEAD`.split.length).to_s
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

# TODO: Shadow these until replaced with APP[] in env/setup/tasks files
RUBY_SO = RbConfig::CONFIG['RUBY_SO_NAME']
APP['RUBY_SO'] = RbConfig::CONFIG['RUBY_SO_NAME']
RUBY_V = RbConfig::CONFIG['ruby_version']
APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']
APP['SHOES_RUBY_ARCH'] = RbConfig::CONFIG['arch']

# default exts, gems & locations to build and include - replace with custom.yaml
APP['GEMLOC'] = ""
APP['EXTLOC'] = ""
APP['EXTLIST'] = []
APP['GEMLIST'] = []
APP['Bld_Tmp'] = 'tmp'
APP['Bld_Pre'] = ENV['NFS_ALTP'] if ENV['NFS_ALTP']

if File.exists? "build_target"
  CROSS = true
  File.open('build_target','r') do |f|
    str = f.readline
    TGT_ARCH = str.split('=')[1].strip
    # is the build output directory outside the shoes3 dir
    if APP['Bld_Pre']
      TGT_DIR = APP['Bld_Pre']+TGT_ARCH
    else
      TGT_DIR = TGT_ARCH
    end
    mkdir_p "#{TGT_DIR}"
    BLD_ARGS = {}
    # This allows short circuiting the need to load setup and env (and pkg-config overhead)
    # Exprimental - no visible performance gain. see make/linux/minlin/env.rb
    if File.exists? "#{TGT_DIR}/build.yaml"
      $stderr.puts "loading building args"
      thsh = YAML.load_file("#{TGT_DIR}/build.yaml")
      thsh.each {|k,v| BLD_ARGS[k] = v} 
      HAVE_BLD = true
    else 
      HAVE_BLD = false
    end
  end
else
  CROSS = false
  if ARGV.length == 0 # bare rake w/o a setup called earlier
    plt = case RUBY_PLATFORM
      when /darwin/ then 'osx'
      when /mingw/  then 'win32'
      when /linux/  then 'linux'
    end
    $stderr.puts "Please Select a #{plt}:setup: target from the"
    $stderr.puts "  list shown by 'rake -T'"
  end
  TGT_DIR = 'unknown'
end


BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
#CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
#CLEAN.include ["req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
CLEAN.include ["#{TGT_DIR}/libshoes.dll", "#{TGT_DIR}/*shoes.exe", 
    "#{TGT_DIR}/libshoes.so","#{TGT_DIR}/shoes", "#{TGT_DIR}/shoes-bin",
    "#{TGT_DIR}/*.app", "#{TGT_DIR}/#{APP['Bld_tmp']}/**/*.o"]
CLOBBER.include ["#{TGT_DIR}/.DS_Store", "#{TGT_DIR}", "build_target", "cshoes", "shoes/**/*.o"]

# for Host building for target
case RUBY_PLATFORM
when /mingw/
  if CROSS
    require File.expand_path("make/win32/#{TGT_ARCH}/env")
    require File.expand_path("make/win32/#{TGT_ARCH}/tasks")
    require File.expand_path("make/win32/#{TGT_ARCH}/stubs")
    require File.expand_path("make/win32/#{TGT_ARCH}/setup")
    require File.expand_path("make/gems")
    require File.expand_path('make/subsys')
  else
    # just enough to do a rake w/o target 
    require File.expand_path('make/win32/none/env.rb')
    require File.expand_path('make/win32/none/tasks.rb')
  end
  Builder = MakeMinGW
  NAMESPACE = :win32

when /darwin/
  if CROSS
    # Building tight shoes on OSX for OSX
    require File.expand_path("make/darwin/#{TGT_ARCH}/env")
    require File.expand_path("make/darwin/#{TGT_ARCH}/tasks")
    require File.expand_path("make/darwin/#{TGT_ARCH}/stubs")
    require File.expand_path("make/darwin/#{TGT_ARCH}/setup")
    require File.expand_path("make/gems")
    require File.expand_path("make/subsys")
  else
    # just enough to do a rake w/o target 
    require File.expand_path('make/darwin/none/env')
    require File.expand_path('make/darwin/none/tasks')
  end
  Builder = MakeDarwin
  NAMESPACE = :osx
  
when /linux/
  if CROSS
    case TGT_ARCH
    when /x86_64-linux/
      require File.expand_path('make/linux/x86_64-linux/env')
      require File.expand_path('make/linux/x86_64-linux/tasks')
      require File.expand_path('make/linux/x86_64-linux/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /i686-linux/
      require File.expand_path('make/linux/i686-linux/env')
      require File.expand_path('make/linux/i686-linux/tasks')
      require File.expand_path('make/linux/i686-linux/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /pi2/
      require File.expand_path('make/linux/pi2/env')
      require File.expand_path('make/linux/pi2/tasks')
      require File.expand_path('make/linux/pi2/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /xarmv6hf/
      require File.expand_path('make/linux/xarm6hf/env')
      require File.expand_path('make/linux/xarm6hf/tasks')
      require File.expand_path('make/gems')
    when /xwin7/
      require File.expand_path('make/linux/xwin7/env')
      require File.expand_path('make/linux/xwin7/tasks')
      require File.expand_path('make/linux/xwin7/stubs')
      require File.expand_path('make/linux/xwin7/setup')
      require File.expand_path('make/linux/xwin7/packdeps')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
   when /xmsys2/
      require File.expand_path('make/linux/xmsys2/env')
      require File.expand_path('make/linux/xmsys2/tasks')
      require File.expand_path('make/linux/xmsys2/stubs')
      require File.expand_path('make/linux/xmsys2/packdeps')
      require File.expand_path('make/linux/xmsys2/setup')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
   when /minlin/ 
      # This is Loose Shoes setup now known as minlin
      if CROSS && HAVE_BLD  # shortcut
        puts "skipping #{TGT_ARCH} env.rb, setup.rb"
        require File.expand_path('make/linux/minlin/tasks')
        require File.expand_path('make/subsys')
        DLEXT = BLD_ARGS['DLEXT']
        CC = BLD_ARGS['CC']
        SOLOCS = BLD_ARGS['SOLOCS']
        LINUX_CFLAGS = BLD_ARGS['LINUX_CFLAGS']
        LINUX_LDFLAGS = BLD_ARGS['LINUX_LDFLAGS']
        LINUX_LIBS = BLD_ARGS['LINUX_LIBS']
      else 
        puts "Require All minlin:"
        require File.expand_path('make/linux/minlin/env')
        require File.expand_path('make/linux/minlin/tasks')
        require File.expand_path('make/linux/minlin/setup')
        require File.expand_path('make/subsys')
     end
   else
      $stderr.puts "Unknown builder for #{TGT_ARCH}, removing setting"
      rm_rf "build_target" if File.exists? "build_target"
    end
  else
     # just enough to do a rake w/o target - will fail with a decent enough
     # error message
     require File.expand_path('make/linux/none/env')
     require File.expand_path('make/linux/none/tasks')
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
    if CROSS && TGT_DIR != 'minlin'
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


# --------------------------
# tasks depending on Builder = MakeLinux|MakeDarwin|MakeMinGW

desc "Build using your OS setup"
task :build => ["#{NAMESPACE}:build"]


# ------  new build   --------
rtp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
file  "#{rtp}/zzsetup.done" do
  Builder.static_setup SOLOCS
  Builder.copy_gems #used to be common_build, located in make/gems.rb
  Builder.setup_system_resources
  touch "#{rtp}/zzsetup.done"
  $stderr.puts "Build Products in #{rtp}"
end

SubDirs = ["#{rtp}/zzbase.done", "#{rtp}/http/zzdownload.done",
   "#{rtp}/plot/zzplot.done", "#{rtp}/console/zzconsole.done", 
   "#{rtp}/types/zzwidgets.done", "#{rtp}/native/zznative.done"]
    
# Windows doesn't use console - don't try to build it. Delete from dependcies
case TGT_DIR
  when /win7/, /xwin7/, /msys2/, /xmsys2/
    SubDirs.delete("#{rtp}/console/zzconsole.done")
end

file "#{TGT_DIR}/libshoes.#{DLEXT}" => ["#{rtp}/zzsetup.done", "shoes/types/types.h",
    "shoes/version.h"] + SubDirs do
  # need to compile version.c -> .o (and create the verion.h for every build)
  sh "#{CC} -o #{rtp}/version.o -I. -c #{LINUX_CFLAGS} shoes/version.c"
  Builder.new_so "#{TGT_DIR}/libshoes.#{DLEXT}"
end

if CROSS # i.e. most platforms. 
  # dependency is set in subsys.rb
  task :update_man => ["#{TGT_DIR}/static/manual-en.txt"]
  task :update_shoes_rb => ["#{TGT_DIR}/lib/shoes.rb"]
  rbdeps = FileList["#{TGT_DIR}/lib/shoes/*.rb"]
  task :update_shoes_internals => rbdeps
end

# rake build (or just rake) will get here. 
task :new_build => "#{TGT_DIR}/libshoes.#{DLEXT}"  do
  # We can link shoes now - this will be done via a Builder call
  # because it's platform specific.
  Builder.new_link "#{TGT_DIR}/shoes"
  if CROSS
    # check if the manual, or the shoes ruby code was changed an
    # needs to be moved to the 'setup' (TGT_DIR)
    # after 
    Rake::Task[:update_man].invoke
    Rake::Task[:update_shoes_rb].invoke
    Rake::Task[:update_shoes_internals].invoke
  end
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

def cc(t)
  $stderr.puts "compiling #{t}"
  sh "#{CC} -I. -c -o #{t.name} #{LINUX_CFLAGS} #{t.source}"
end

rule ".o" => ".m" do |t|
  cc t
end

rule ".o" => ".c" do |t|
  cc t
end

task :installer => ["#{NAMESPACE}:installer"]

namespace :osx do
  namespace :setup do
    #desc "Setup to build Shoes for 10.10+"
    #task :yosemite do
    #  sh "echo 'TGT_ARCH=yosemite' >build_target"
    #end

    desc "Setup to build Shoes for 10.9+ from 10.10+"
    task :xmavericks do
      sh "echo 'TGT_ARCH=xmavericks' >build_target"
    end

    #desc "Setup to build Shoes for 10.9+ from 10.9"
    #task :mavericks do
    #  sh "echo 'TGT_ARCH=mavericks' >build_target"
    #end

    #desc "Setup to build for 10.6+ from 10.6"
    #task :snow do
    #  sh "echo 'TGT_ARCH=snow' >build_target"
    #end

    #desc "Downshift Build 10.6 from 10.9"
    #task "xsnow" do
    #  sh "echo 'TGT_ARCH=xsnow' >build_target"
    #end

  end

  task :build => [:new_build]

  task :installer do
    Builder.make_installer
  end
end

namespace :win32 do

  namespace :setup do
    desc "Winodws build with devkit"
    task :win7 do
      sh "echo TGT_ARCH=win7 >build_target"
    end

    desc "Windows build with msys2"
    task :msys2 do
      sh "echo TGT_ARCH=msys2 >build_target"
    end    
  end

  task :build => ["build_target", :new_build]

  task :installer do
    Builder.make_installer
  end
end

# get here when rake decides it's running on a linux system
namespace :linux do

  namespace :setup do
    desc "build non-portable linux"
    task :minlin do
      sh "echo 'TGT_ARCH=minlin' >build_target"
    end
    
    #desc "Cross compile to arm6hf - advanced users"
    task :xarm6hf do
     sh "echo 'TGT_ARCH=xarmv6hf' >build_target"
    end
    
    desc "Native arm build - pi 2+"
    task :pi2 do
      sh "echo 'TGT_ARCH=pi2' >build_target"
    end
    
    #desc "Cross compile for msys2 deps (mingw)"
    task :xmsys2 do
      puts "Cross compile newer deps (mingw)"
      sh "echo 'TGT_ARCH=xmsys2' >build_target"
    end

    desc "Cross compile with  MingW32"
    task :xwin7 do
      puts "Cross compile for Windows MingW32"
      sh "echo 'TGT_ARCH=xwin7' >build_target"
    end

    desc "chroot build for i686 (32bit linux)"
    task :i686_linux do
      puts "Cross complile for i686-linux"
      sh "echo 'TGT_ARCH=i686-linux' >build_target"
    end

    desc "chroot build for x86_64 (64bit linux)"
    task :x86_64_linux do
      puts "Cross complile for x86_64-linux"
      sh "echo 'TGT_ARCH=x86_64-linux' >build_target"
    end

  end
  
  task :build => [:new_build]

  task :installer do
    Builder.make_installer
  end

end
