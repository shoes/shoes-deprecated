require 'rexml/document'
require 'fileutils'
include FileUtils

# locate ~/.shoes
require 'tmpdir'

lib_dir = nil
homes = []
homes << [ENV['HOME'], File.join( ENV['HOME'], '.shoes' )] if ENV['HOME']
homes << [ENV['APPDATA'], File.join( ENV['APPDATA'], 'Shoes' )] if ENV['APPDATA']
homes.each do |home_top, home_dir|
  next unless home_top
  if File.exists? home_top
    lib_dir = home_dir
    break
  end
end
LIB_DIR = lib_dir || File.join(Dir::tmpdir, "shoes")
SITE_LIB_DIR = File.join(LIB_DIR, '+lib')
GEM_DIR = File.join(LIB_DIR, '+gem')

mkdir_p(LIB_DIR)
$:.unshift SITE_LIB_DIR
$:.unshift GEM_DIR

require 'rbconfig'
Config::CONFIG['libdir'] = GEM_DIR
Config::CONFIG['sitelibdir'] = SITE_LIB_DIR

require 'rubygems'
if Object::const_defined? :OpenURI
  Object.send :remove_const, :OpenURI
  module Kernel
    remove_method :open
    alias open open_uri_original_open
  end
end
require 'rubygems/open-uri'
require 'rubygems/installer'
class << Gem::Ext::ExtConfBuilder
  alias_method :make__, :make
  def self.make(dest_path, results)
    raise unless File.exist?('Makefile')
    mf = File.read('Makefile')
    mf = mf.gsub(/^INSTALL\s*=\s*\$[^$]*/, "INSTALL = '@$(RUBY) -run -e install -- -vp'")
    mf = mf.gsub(/^INSTALL_PROG\s*=\s*\$[^$]*/, "INSTALL_PROG = '$(INSTALL) -m 0755'")
    mf = mf.gsub(/^INSTALL_DATA\s*=\s*\$[^$]*/, "INSTALL_DATA = '$(INSTALL) -m 0644'")
    File.open('Makefile', 'wb') {|f| f.print mf}
    make__(dest_path, results)
  end
end
class Gem::Installer
  alias_method :install__, :install
  def install(*a)
    install__
  end
end

# STDIN.reopen("/dev/tty") if STDIN.eof?
class NotSupportedByShoes < Exception; end

def gem_reset
  if Gem.const_defined? :ConfigFile
    Gem.configuration = Gem::ConfigFile.new(:gemhome => GEM_DIR, :gempath => GEM_DIR)
  end
  Gem.use_paths(GEM_DIR, [GEM_DIR])
  Gem.source_index.refresh!
end
def start(name, author, url, note, steps, &blk)
  GEM_DIR.replace File.join(GEM_DIR, name)
  puts "** Using #{GEM_DIR} for temporary storage"
  mkdir_p(GEM_DIR)
  $:.unshift GEM_DIR

  $stderr = StringIO.new
  begin
    Gem.manage_gems
    class << Gem; attr_accessor :loaded_specs end

    gem_reset
    install_sources
    require_gem 'sources'

    title = "#{name}, a shoes by #{author}"
    puts
    puts title
    puts "=" * title.length
    puts note
    puts
    puts "[*] PLEASE NOTE: This script runs all kinds of code from off the Internet, all packaged together"
    puts "  by #{author}, the shoeser.  Either you trust this person or you've read this script and you"
    puts "  know what you're getting into, right?"
    puts
    loop do
      print "Proceed (or L to list this script's actions)? [y/N/l] "
      case gets.strip.downcase
      when 'y'
        break
      when 'l'
        puts steps
        puts "(*) See #{url} for full details."
        puts
      else
        abort(">|< POP!")
      end
    end
    blk.call
  rescue Exception => e
    puts "#{e.class}: #{e.message}\n  #{e.backtrace * "\n  "}"
  end
end
def source(uri)
  puts "** Getting gems from #{uri}..."
  Gem.sources.clear
  Gem.sources << uri
end
def libdir(path)
  path = File.expand_path(path)
  puts "** Adding #{path} to the $LOAD_PATH..."
  $:.unshift path
end
def fetch_gem(name, version = nil, &blk)
  if Gem.source_index.find_name(name, version).empty?
    puts "** Fetching #{name} #{version}"

    installer = Gem::Installer.new(:include_dependencies => true)
    installer.install(name, version || Gem::Requirement.default, true)
    gem_reset
  end
  gem = Gem.source_index.find_name(name, version).first
  Gem.activate(gem.name, true, "= #{gem.version}")
  if blk
    puts "** Entering #{gem.full_gem_path}..."
    Dir.chdir(gem.full_gem_path, &blk)
  end
end
def svn(dir, save_as = nil, &blk)
  dir.gsub! /(.)\/*$/, '\1/'
  if save_as.nil? or save_as.empty?
    save_as = File.join(GEM_DIR, 'svn', '1')
    save_as.succ! while File.exists? save_as
  elsif save_as.index(GEM_DIR) != 0
    save_as = File.join(GEM_DIR, 'svn', save_as)
  end
  mkdir_p(save_as)
  puts "** Pulling down #{dir}..."
  svnuri = URI.parse(dir)
  case svnuri.scheme
  when "http", "https"
    REXML::Document.new(svnuri.open { |f| f.read }).
      each_element("/svn/index/*") do |ele|
        fname, href = ele.attributes['name'], ele.attributes['href']
        case ele.name
        when "file"
          puts "- #{dir}#{href}"
          URI.parse("#{dir}#{href}").open do |f|
            File.open(File.join(save_as, fname), 'wb') do |f2|
              f2 << f.read(16384) until f.eof?
            end
          end
        when "dir"
          svn("#{dir}#{href}", File.join(save_as, fname))
        end
      end
  else
    raise NotSupportedByShoes, "Only HTTP addresses are supported by Shoes's Subversion module."
  end
  if blk
    Dir.chdir(save_as, &blk)
  end
end
