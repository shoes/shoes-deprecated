require 'rubygems'
require 'rake'
require 'rake/clean'
if RUBY_VERSION != '1.8.7'
require_relative 'platform/skel'
else
require File.join(File.dirname(__FILE__), 'platform/skel')
end
require 'fileutils'
require 'find'
require 'yaml'
include FileUtils

APP = YAML.load_file(File.join(ENV['APP'] || ".", "app.yaml"))
APPNAME = APP['name']
RELEASE_ID, RELEASE_NAME = APP['version'], APP['release']
NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
SONAME = 'shoes'

require 'rake/testtask'
Rake::TestTask.new do |t|
  t.pattern = "test/*_test.rb" 
end 

require 'bundler'
Bundler::GemHelper.install_tasks

GIT = ENV['GIT'] || "git"
REVISION = (`#{GIT} rev-list HEAD`.split.length + 1).to_s
VERS = ENV['VERSION'] || "0.r#{REVISION}"
PKG = "#{NAME}-#{VERS}"
APPARGS = APP['run']
FLAGS = %w[DEBUG VIDEO]
VLC_VERSION = (RUBY_PLATFORM =~ /win32/ ? "0.8": `vlc --version 2>/dev/null`.split[2])
VLC_0_8 = VLC_VERSION !~ /^0\.9/

BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "dist"]

RUBY_SO = Config::CONFIG['RUBY_SO_NAME']
RUBY_V = Config::CONFIG['ruby_version']
RUBY_1_9 = (RUBY_V =~ /^1\.9/)
if RUBY_1_9
  $: << "."
end

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

case RUBY_PLATFORM
when /mingw/
  require File.expand_path('rakefile_mingw')
  Builder = MakeMinGW
when /darwin/
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
    %w[VIDEO DEBUG].each { |x| f << " +#{x.downcase}" if ENV[x] }
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

# Here's all of the deps that Shoes needs!
#
# The key is the directory of each dependency. The values are hashes which
# have some elements:
#   :url => The url where that dep can be downloaded from
#   :config => the option that needs to be passed to ./configure
#   :final_file => Basically, if this is built, we dont need to rebuild the dep
#
# That's it!
deps_list = {
  "deps/src/pkg-config-0.20" => {
    :url => "http://pkg-config.freedesktop.org/releases/pkg-config-0.20.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/libpng-1.4.1" => {
    :url => "http://sourceforge.net/projects/libpng/files/libpng14/older-releases/1.4.1/libpng-1.4.1.tar.gz/download", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/libpng14.14.dylib",
  },
  "deps/src/giflib-4.1.6" => {
    :url => "http://sourceforge.net/projects/giflib/files/giflib%204.x/giflib-4.1.6/giflib-4.1.6.tar.gz/download", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/libgif.dylib",
  },
  "deps/src/gettext-0.17" => {
    :url => "http://ftp.gnu.org/gnu/gettext/gettext-0.17.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/libgettextlib.dylib",
  },
  "deps/src/libiconv-1.13" => {
    :url => "http://ftp.gnu.org/gnu/libiconv/libiconv-1.13.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/libiconv.dylib",
  },
  "deps/src/pixman-0.18.0" => {
    :url => "http://cgit.freedesktop.org/pixman/snapshot/pixman-0.18.0.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/libpixman-1.dylib",
    :autogen => true
  },
  "deps/src/portaudio" => {
    :url => "http://portaudio.com/archives/pa_stable_v19_20071207.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --disable-mac-universal",
    :final_file => "deps/lib/libportaudio.dylib",
  },
  "deps/src/glib-2.28.1" => {
    :url => "http://ftp.gnome.org/pub/gnome/sources/glib/2.28/glib-2.28.1.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
    :final_file => "deps/lib/glib-2.0",
  },
  "deps/src/cairo-1.8.10" => {
    :url => "http://cairographics.org/releases/cairo-1.8.10.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-quartz=yes --enable-quartz-font=yes --enable-xlib=no",
    :final_file => "deps/lib/libcairo.dylib",
  },
  "deps/src/pango-1.28.0" => {
    :url => "http://ftp.gnome.org/pub/GNOME/sources/pango/1.28/pango-1.28.0.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --with-x=no",
    :final_file => "deps/lib/libpango-1.0.dylib",
  },
  "deps/src/jpeg-8a" => {
    :url => "http://ijg.org/files/jpegsrc.v8a.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-shared",
    :final_file => "deps/lib/libjpeg.dylib",
  },
  "deps/src/ruby-1.9.1-p378" => {
    :url => "http://ftp.ruby-lang.org//pub/ruby/1.9/ruby-1.9.1-p378.tar.gz",
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-shared",
    :final_file => "deps/lib/libruby.dylib",
  },
  "deps/src/rubygems-1.3.6.tgz" => {
    :url => "http://production.cf.rubygems.org/rubygems/rubygems-1.3.6.tgz",
    :config => "",
  },
}

# Note: right now, this just works due to the ordering, but really, this should
# be encoded in the tasks: pango needs cairo, and glib. Otherwise it wont build.

# First we want to download all the deps
deps_steps = deps_list.keys

# Then let's build pkg-config
deps_steps << "deps/bin/pkg-config"

# Next set up the environment variables
deps_steps << "set-env"

# Next is all of the deps
deps_steps += deps_list.reject{|k| k =~ /pkg-config/ or k =~ /rubygems/ }.map{|k, v| deps_list[k][:final_file]}

# Then comes rubygems...
deps_steps << "install-rubygems"

# Then finally, rake
deps_steps << "install-rake"

desc "Build dependencies on Snow Leopard"
task "deps" => deps_steps do
  vputs "done building deps!"
end

file "deps/bin/pkg-config" => "deps/src/pkg-config-0.20" do
  # Let's build pkg-config
  cd "deps/src/pkg-config-0.20" do
    sh "./configure #{deps_list['deps/src/pkg-config-0.20'][:config]}"
    sh "make && make install"
  end
end

deps_dir = File.dirname(__FILE__) + "/deps"

deps_list.each do |k, opts|
  file opts[:final_file] => k do
    cd k do
      # sometimes, we need to run autogen.sh to make our configure file.
      if opts[:autogen]
        sh "sh autogen.sh"
      end

      # Standard three commands to compile all of this.
      sh "./configure #{opts[:config]}"
      sh "make && make install"
    end
  end
end

task "install-rubygems" => "#{deps_dir}/bin/gem" do
  # I'm not sure this is still important with ruby1.9.
  cd "#{deps_dir}/src/rubygems-1.3.6" do
    sh "#{deps_dir}/bin/ruby setup.rb"
  end
end

task "install-rake" do
  # Same thing: dunno on 1.9

  # Gotta find a way for this to only run if it hasn't been yet.
  # Probably set a dep based on where it puts the gem, I don't know off the top
  # of my head and it's 3am.
  sh "#{deps_dir}/bin/gem install rake"
end

# Sets up a bunch of environment variables
task "set-env" do
  ENV['MACOSX_DEPLOYMENT_TARGET'] = "10.6"
  ENV['PATH'] = "#{deps_dir}/bin:#{ENV['PATH']}"
  ENV['CFLAGS'] = "-I#{deps_dir}/include -w"
  ENV['LDFLAGS'] = "-L/#{deps_dir}/lib"
  ENV['PKG_CONFIG_PATH'] = "#{deps_dir}/lib/pkgconfig"
end


# Let's use mechanize to download the files we need.
require 'mechanize'
agent = Mechanize.new

# We want to make sure that the deps/src directory gets built: that's where
# all of the source code for our dependencies goes
directory "deps/src"

# These tasks all download the source for our dependencies.
#
# We want to loop through the deps list.
deps_list.each do |name, opts|

  # Each file depends on having the src directory built
  file name => "deps/src" do

    # Grab the URL we need to download from.
    url = opts[:url]

    # get just the filename
    url =~ /([^\/]+(?:(?:\.tar\.gz)|(?:\.tgz)))/
    f = $1

    cd "deps/src" do
      vputs "downloading #{url}"
      agent.get(url).save

      # For some reason, the pixman downloads with quotes.
      if File.exists?("\"pixman-0.18.0.tar.gz\"")
        mv "\"pixman-0.18.0.tar.gz\"", "pixman-0.18.0.tar.gz"
      end

      # unzip!
      sh "tar -xzf #{f}"
    end
  end
end
