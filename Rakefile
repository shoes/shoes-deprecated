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
APPNAME = APP['name']
RELEASE_ID, RELEASE_NAME = APP['version'], APP['release']
NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
SONAME = 'shoes'

# Like puts, but only if we've --trace'd
def vputs(str)
  puts str if Rake.application.options.trace
end

GIT = ENV['GIT'] || "git"
#REVISION = (`#{GIT} rev-list HEAD`.split.length + 1).to_s
#VERS = ENV['VERSION'] || "0.r#{REVISION}"
REVISION = "3.2"
VERS = "3.2"
PKG = "#{NAME}-#{VERS}"
APPARGS = APP['run']
FLAGS = %w[DEBUG]



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
  ENV['CAIRO_CFLAGS'] = '-I/usr/local/Cellar/cairo/1.10.2/include/cairo'
  ENV['SHOES_DEPS_PATH'] = '/usr/local'
end

if File.exists? "crosscompile"
  CROSS = true
  File.open('crosscompile','r') do |f|
    str = f.readline
    TGT_ARCH = str.split('=')[1].strip
    TGT_DIR = TGT_ARCH
  end
else
  CROSS = false
  TGT_DIR = 'dist'
end

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
#CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
CLEAN.include ["req/**/#{BIN}", "#{TGT_DIR}", "*.app"]

# for Host building for Host:
case RUBY_PLATFORM
when /mingw/
  require File.expand_path('rakefile_mingw')
  Builder = MakeMinGW
  NAMESPACE = :win32
when /darwin/
  osx_bootstrap_env
  require File.expand_path('make/darwin/env')
  require_relative "make/darwin/homebrew"

  task :stub do
    ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
    sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
  end
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
    when /xmsw32/
      require File.expand_path('make/xmsw32/env')
      require File.expand_path('make/xmsw32/tasks')
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
task :package => [:installer]

task :build_os => [:build_skel, "#{TGT_DIR}/#{NAME}"]


task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << %{#define SHOES_RELEASE_ID #{RELEASE_ID}\n#define SHOES_RELEASE_NAME "#{RELEASE_NAME}"\n#define SHOES_REVISION #{REVISION}\n#define SHOES_BUILD_DATE "#{Time.now.strftime("%Y%m%d")}"\n#define SHOES_PLATFORM "#{SHOES_RUBY_ARCH}"\n}
    if CROSS  
      f << '#define SHOES_STYLE "TIGHT_SHOES"'
    else
      f << '#define SHOES_STYLE "LOOSE_SHOES"'
    end
  end
end

# Left for historical reasons (aka OSX)
task "#{TGT_DIR}/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION}) [#{SHOES_RUBY_ARCH} Ruby#{RUBY_V}]}
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

task :pre_build do
  Builder.pre_build
end

# first refactor ; build calls platform namespaced build;
# for now, each of those calls the old build method.
task :old_build => [:pre_build, :build_os] do
  Builder.common_build
  Builder.copy_deps_to_dist
  Builder.setup_system_resources
end

desc "Install Shoes in your  ~/.shoes Directory"
task  :install do
  if CROSS 
     puts "Sorry. You can't do an install of your source built Shoes"
     puts "when crosscompiling is setup. Think about the confused children."
  else
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
  namespace :deps do
    task :install => "homebrew:install"
    namespace :homebrew do
      desc "Install OS X dependencies using Homebrew"
      task :install => [:customize, :install_libs, :uncustomize]

      task :install_libs do
        brew = Homebrew.new
        brew.universal if ENV['SHOES_OSX_ARCH'] == "universal"
        brew.install_packages
      end

      task :customize do
        brew = Homebrew.new
        brew.universal if ENV['SHOES_OSX_ARCH'] == "universal"
        brew.add_custom_remote
        brew.add_custom_formulas
      end

      task :uncustomize do
        brew = Homebrew.new
        brew.universal if ENV['SHOES_OSX_ARCH'] == "universal"
        brew.remove_custom_formulas
        brew.remove_custom_remote
      end
   end
  end

  task :build => ["build_tasks:pre_build", :build_skel, "dist/#{NAME}", "dist/VERSION.txt", "build_tasks:build"]

  namespace :build_tasks do

    task :build => [:common_build, :copy_deps_to_dist, :change_install_names, :copy_files_to_dist, :setup_system_resources, :verify]

    # Make sure the installed ruby is capable of this build
    task :check_ruby_arch do
      build_arch, ruby_arch = [OSX_ARCH, Config::CONFIG['ARCH_FLAG']].map {|s| s.split.reject {|w| w.include?("arch")}}
      if build_arch.length > 1 and build_arch.sort != ruby_arch.sort
        abort("To build universal shoes, you must first install a universal ruby")
      end
    end

    task :pre_build => :check_ruby_arch

    def copy_ext_osx xdir, libdir
      Dir.chdir(xdir) do
        `ruby extconf.rb; make`
      end
      copy_files "#{xdir}/*.bundle", libdir
    end

    task :common_build do
      mkdir_p "dist/ruby"
      cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "dist/ruby/lib"
      unless ENV['STANDARD']
        %w[soap wsdl xsd].each do |libn|
          rm_rf "dist/ruby/lib/#{libn}"
        end
      end
      %w[req/ftsearch/lib/* req/rake/lib/*].each do |rdir|
        FileList[rdir].each { |rlib| cp_r rlib, "dist/ruby/lib" }
      end
      %w[req/binject/ext/binject_c req/ftsearch/ext/ftsearchrt req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
        each { |xdir| copy_ext_osx xdir, "dist/ruby/lib/#{SHOES_RUBY_ARCH}" }

      gdir = "dist/ruby/gems/#{RUBY_V}"
      {'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
        spec = eval(File.read("req/#{gemn}/gemspec"))
        mkdir_p "#{gdir}/specifications"
        mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
        FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
        mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
        FileList["req/#{gemn}/ext/*"].each { |elib| copy_ext_osx elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
        cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
      end
    end

    def dylibs_to_change lib
      `otool -L #{lib}`.split("\n").inject([]) do |dylibs, line|
        if  line =~ /^\S/ or line =~ /System|@executable_path|libobjc/
          dylibs
        else
          dylibs << line.gsub(/\s\(compatibility.*$/, '').strip
        end
      end
    end

    task :change_install_names do
      cd "dist" do
        ["#{NAME}-bin", "pango-querymodules", *Dir['*.dylib'], *Dir['pango/modules/*.so']].each do |f|
          sh "install_name_tool -id @executable_path/#{File.basename f} #{f}"
          dylibs = dylibs_to_change(f)
          dylibs.each do |dylib|
            sh "install_name_tool -change #{dylib} @executable_path/#{File.basename dylib} #{f}"
          end
        end
      end
    end

    task :copy_pango_modules_to_dist do
      modules_file = `brew --prefix`.chomp << '/etc/pango/pango.modules'
      modules_path = File.open(modules_file) {|f| f.grep(/^# ModulesPath = (.*)$/){$1}.first}
      mkdir_p 'dist/pango'
      cp_r modules_path, 'dist/pango'
      cp `which pango-querymodules`.chomp, 'dist/'
    end

    task :copy_deps_to_dist => :copy_pango_modules_to_dist do
      # Generate a list of dependencies straight from the generated files.
      # Start with dependencies of shoes-bin and pango-querymodules, and then
      # add the dependencies of those dependencies.
      dylibs = dylibs_to_change("dist/#{NAME}-bin")
      dylibs.concat dylibs_to_change("dist/pango-querymodules")
      dupes = []
      dylibs.each do |dylib|
        dylibs_to_change(dylib).each do |d|
          if dylibs.map {|lib| File.basename(lib)}.include?(File.basename(d))
            dupes << d
          else
            dylibs << d
          end
        end
      end
      dylibs.each {|libn| cp "#{libn}", "dist/"}
    end

    task :copy_files_to_dist do
      if ENV['APP']
        if APP['clone']
          sh APP['clone'].gsub(/^git /, "#{GIT} --git-dir=#{ENV['APP']}/.git ")
        else
          cp_r ENV['APP'], "dist/app"
        end
        if APP['ignore']
          APP['ignore'].each do |nn|
            rm_rf "dist/app/#{nn}"
          end
        end
      end

      cp_r  "fonts", "dist/fonts"
      cp_r  "lib", "dist/lib"
      cp_r  "samples", "dist/samples"
      cp_r  "static", "dist/static"
      cp    "README.md", "dist/README.txt"
      cp    "CHANGELOG", "dist/CHANGELOG.txt"
      cp    "COPYING", "dist/COPYING.txt"
    end

    task :setup_system_resources do
      rm_rf "#{APPNAME}.app"
      mkdir "#{APPNAME}.app"
      mkdir "#{APPNAME}.app/Contents"
      cp_r "dist", "#{APPNAME}.app/Contents/MacOS"
      mkdir "#{APPNAME}.app/Contents/Resources"
      mkdir "#{APPNAME}.app/Contents/Resources/English.lproj"
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/App.icns\""
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/Contents/Resources/App.icns\""
      rewrite "platform/mac/Info.plist", "#{APPNAME}.app/Contents/Info.plist"
      cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
      rewrite "platform/mac/pangorc", "#{APPNAME}.app/Contents/MacOS/pangorc"
      cp "platform/mac/command-manual.rb", "#{APPNAME}.app/Contents/MacOS/"
      rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
      rewrite "platform/mac/shoes", "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod_R 0755, "#{APPNAME}.app/Contents/MacOS/pango-querymodules"
      # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
      `echo -n 'APPL????' > "#{APPNAME}.app/Contents/PkgInfo"`
    end
  end

  desc "Verify the build products"
  task :verify => ['verify:sanity', 'verify:lib_paths']

  namespace :verify do
    def report_error message
      STDERR.puts "BUILD ERROR: " + message
    end

    task :sanity do
      report_error "No #{APPNAME}.app file found" unless File.exist? "#{APPNAME}.app"
      [NAME, "#{NAME}-launch", "#{NAME}-bin"].each do |f|
        report_error "No #{f} file found" unless File.exist? "#{APPNAME}.app/Contents/MacOS/#{f}"
      end
    end

    task :lib_paths do
      cd "#{APPNAME}.app/Contents/MacOS" do
        errors = []
        ["#{NAME}-bin", "pango-querymodules", *Dir['*.dylib'], *Dir['pango/modules/*.so']].each do |f|
          dylibs = dylibs_to_change(f)
          dylibs.each do |dylib|
            errors << "Suspect library path on #{f}:\n  #{dylib}\n  (check with `otool -L #{File.expand_path f}`)"
          end
        end
        errors.each {|e| report_error e}
      end
    end
  end

  task :make_app do
    # Builder.make_app "dist/#{NAME}"
    bin = "dist/#{NAME}-bin"
    rm_f "dist/#{NAME}"
    rm_f bin
    sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{OSX_ARCH}"
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
    sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/pango-querymodules"
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

# get here when rake decides it's running on a linux system
namespace :linux do

  namespace :setup do
    desc "Cross compile to Raspberry pi"
    task :crosspi do
      puts "Cross compile to Raspberry setup"
      sh "echo 'TGT_ARCH=xarmv6-pi' >crosscompile"
    end

    desc "Cross compile to Windows Native GUI"
    task :msw32 do
      puts "Cross compile for WIN32"
      sh "echo 'TGT_ARCH=xmsw32' >crosscompile"
    end

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
