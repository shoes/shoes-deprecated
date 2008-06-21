#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
ARGV.delete_if { |x| x =~ /-psn_/ }

require 'open-uri'
require 'optparse'
require 'resolv-replace'
require 'shoes/inspect'
if Object.const_defined? :Shoes
  require 'shoes/image'
  require 'shoes/cache'
end
 
class Range 
  def rand 
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv) 
  end 
end

unless Time.respond_to? :today
  def Time.today
    t = Time.now
    t - (t.to_i % 86400)
  end
end

class Canvas
  [:height, :width, :scroll].each do |m|
    define_method(m) { style[m] } unless method_defined? m
    define_method("#{m}=") { |v| style(m => v) }
  end
end

class Shoes
  VERSION = "Raisins"

  MAIN = Object.new
  def MAIN.to_s
    "(shoes)"
  end

  BINDING = MAIN.instance_eval { binding }

  NotFound = proc do
    para "404 NOT FOUND, GUYS!"
  end
 
  @mounts = []

  OPTS = OptionParser.new do |opts|
    opts.banner = "Usage: shoes [options] (app.rb or app.shy)"
    
    opts.on("-m", "--manual",
            "Open the built-in manual.") do
      show_manual
    end

    opts.on("-s", "--shy DIRECTORY",
            "Compress a directory into a Shoes YAML (SHY) archive.") do |s|
      make_shy(s)
    end

    opts.on("-p", "--package",
            "Package a Shoes app for Windows, OS X and Linux.") do |s|
      make_pack
    end

    opts.on("-g", "--gem",
            "Passes commands to RubyGems.") do
      require 'shoes/setup'
      require 'rubygems/gem_runner'
      Gem::GemRunner.new.run(ARGV)
      raise SystemExit, ""
    end

    opts.on_tail("-v", "--version", "Display the version info.") do
      raise SystemExit, "shoes #{Shoes::RELEASE_NAME.downcase} (0.r#{Shoes::REVISION})"
    end

    opts.on_tail("-h", "--help", "Show this message") do
      raise SystemExit, opts.to_s
    end
  end

  class SettingUp < StandardError; end

  @setups = {}

  def self.setup &blk
    require 'shoes/setup'
    line = caller[0]
    return if @setups[line]
    script = line[/^(.+?):/, 1]
    set = Shoes::Setup.new(script, &blk)
    @setups[line] = true
    unless set.no_steps?
      raise SettingUp
    end
  end

  def self.show_selector
    fname = ask_open_file
    Shoes.load(fname) if fname
  end

  def self.make_shy(s)
    require 'shoes/shy'
    Shoes.app(&ShyMake.call(s))
  end

  def self.make_pack
    require 'shoes/pack'
    Shoes.app(:width => 500, :height => 380, :resizable => false, &PackMake)
  end

  def self.show_manual
    require 'shoes/search'
    require 'shoes/help'
    Shoes.app(&Shoes::Help)
  end

  def self.show_log
    require 'shoes/log'
    Shoes.app do
      extend Shoes::LogWindow
      setup
    end
  end

  def self.mount(path, meth, &blk)
    @mounts << [path, meth || blk]
  end

  SHOES_URL_RE = %r!^@([^/]+)(.*)$! 

  def self.run(path)
    uri = URI(path)
    @mounts.each do |mpath, rout|
      m, *args = *path.match(/^#{mpath}$/)
      if m
        unless rout.is_a? Proc
          rout = rout[0].instance_method(rout[1])
        end
        return [rout, args]
      end
    end
    case uri.path when "/"
      [nil]
    when SHOES_URL_RE
      [proc { eval(URI("http://#$1:53045#$2").read) }]
    else
      [NotFound]
    end
  end

  def self.args!
    if PLATFORM !~ /darwin/
      if ARGV.empty?
        fname = ask_open_file
        if fname
          ARGV << fname
        else
          return false
        end
      end
    end
    OPTS.parse! ARGV
    ARGV[0] or true
  end

  def self.uri(str)
    if str =~ SHOES_URL_RE
      URI("http://#$1:53045#$2")
    else
      URI(str) rescue nil
    end
  end

  def self.load(path)
    uri = Shoes.uri(path)

    case uri
    when URI::HTTP
      str = uri.read
      if str !~ /Shoes\.app/
        Shoes.app do
          eval(uri.read)
        end
      else
        eval(uri.read)
      end
    else
      path = File.expand_path(path.gsub(/\\/, "/"))
      if path =~ /\.shy$/
        require 'shoes/shy'
        base = File.basename(path, ".shy")
        tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
        shy = Shy.x(path, tmpdir)
        Dir.chdir(tmpdir)
        Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
        path = shy.launch
      else
        Dir.chdir(File.dirname(path))
        path = File.basename(path)
      end

      $0.replace path

      code = File.read(path)
      eval(code, Shoes::BINDING, path)
    end
  rescue SettingUp
  end

  def self.url(path, meth)
    Shoes.mount(path, [self, meth])
  end

  class Text
    [:stroke, :fill, :strikecolor, :undercolor, :font, :size, :family, :weight,
     :rise, :kerning, :emphasis, :strikethrough, :stretch, :underline, :variant].each do |m|
      define_method(m) { style[m] }
      define_method("#{m}=") { |v| style[m] = v }
    end
  end

  def Widget.inherited subc
    Shoes.class_eval %{
      def #{subc.to_s[/::(\w+)$/, 1].downcase}(*a, &b)
        a.unshift #{subc}
        widget(*a, &b)
      end
    }
  end
end
