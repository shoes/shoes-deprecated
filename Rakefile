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

# This is the list of dependancies that Shoes needs. The keys are the filenames
# in the deps directory, and the values are the URLs where they can be downloaded.
deps_list = {
  "deps/src/pkg-config-0.20" => {
    :url => "http://pkg-config.freedesktop.org/releases/pkg-config-0.20.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/libpng-1.4.1" => {
    :url => "http://sourceforge.net/projects/libpng/files/libpng14/older-releases/1.4.1/libpng-1.4.1.tar.gz/download", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/giflib-4.1.6" => {
    :url => "http://sourceforge.net/projects/giflib/files/giflib%204.x/giflib-4.1.6/giflib-4.1.6.tar.gz/download", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/gettext-0.17" => {
    :url => "http://ftp.gnu.org/gnu/gettext/gettext-0.17.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/libiconv-1.13" => {
    :url => "http://ftp.gnu.org/gnu/libiconv/libiconv-1.13.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/pixman-0.18.0" => {
    :url => "http://cgit.freedesktop.org/pixman/snapshot/pixman-0.18.0.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/pango-1.28.0" => {
    :url => "http://ftp.gnome.org/pub/GNOME/sources/pango/1.28/pango-1.28.0.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --with-x=no",
  },
  "deps/src/portaudio" => {
    :url => "http://portaudio.com/archives/pa_stable_v19_20071207.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --disable-mac-universal",
  },
  "deps/src/cairo-1.8.10" => {
    :url => "http://cairographics.org/releases/cairo-1.8.10.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-quartz=yes --enable-quartz-font=yes --enable-xlib=no",
  },
  "deps/src/jpeg-8a" => {
    :url => "http://ijg.org/files/jpegsrc.v8a.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-shared",
  },
  "deps/src/glib-2.24.0" => {
    :url => "http://ftp.gnome.org/pub/gnome/sources/glib/2.24/glib-2.24.0.tar.gz", 
    :config => "--prefix=#{File.dirname(__FILE__)}/deps",
  },
  "deps/src/ruby-1.9.1-p378" => {
    :url => "http://ftp.ruby-lang.org//pub/ruby/1.9/ruby-1.9.1-p378.tar.gz",
    :config => "--prefix=#{File.dirname(__FILE__)}/deps --enable-shared",
  },
  "deps/src/rubygems-1.3.6.tgz" => {
    :url => "http://production.cf.rubygems.org/rubygems/rubygems-1.3.6.tgz",
    :config => "",
  },
}

desc "Build dependencies on Snow Leopard"
task "deps" => deps_list.keys do

  #ENV['SOMETHING'] = "HELLO"

  # Everything is unzipped. Time to build.
  cd "deps/src/pkg-config-0.20" do
    sh "./configure #{deps_list['deps/src/pkg-config-0.20'][:config]}"
    sh "make && make install"
  end

  puts "done building deps!"

end

require 'mechanize'
agent = Mechanize.new

directory "deps/src"

deps_list.each do |name, opts|
  file name => "deps/src" do

    url = opts[:url]

    # get just the filename
    url =~ /([^\/]+(?:(?:\.tar\.gz)|(?:\.tgz)))/
    f = $1

    cd "deps/src" do
      puts "downloading #{url}"
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
