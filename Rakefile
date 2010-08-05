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

# Check the environment
def env(x)
  unless ENV[x]
    abort "Your #{x} environment variable is not set!"
  end
  ENV[x]
end

# Subs in special variables
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

def common_build
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
    each { |xdir| copy_ext xdir, "dist/ruby/lib/#{RUBY_PLATFORM}" }

  gdir = "dist/ruby/gems/#{RUBY_V}"
  {'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
    spec = eval(File.read("req/#{gemn}/gemspec"))
    mkdir_p "#{gdir}/specifications"
    mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
    FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
    mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
    FileList["req/#{gemn}/ext/*"].each { |elib| 
      copy_ext elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
    cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
  end
end

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

case RUBY_PLATFORM
when /mingw/
  require 'rakefile_mingw'
when /darwin/
  require 'rakefile_darwin'
when /linux/
  require 'rakefile_linux'
else
  puts 'Sorry, your platform is not supported...'
end
