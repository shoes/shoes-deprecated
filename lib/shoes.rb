#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
ARGV.delete_if { |x| x =~ /-psn_/ }

require 'open-uri'
require 'optparse'
require 'shoes/inspect'
require 'shoes/shy'
if Object.const_defined? :Shoes
  require 'shoes/image'
  require 'shoes/cache'
  require 'shoes/help'
  require 'shoes/log'
end
 
def Object.const_missing c
  Shoes.const_get(c)
end

class Range 
  def rand 
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv) 
  end 
end

class Canvas
  [:height, :width, :scroll].each do |m|
    define_method(m) { style[m] } unless method_defined? m
    define_method("#{m}=") { |v| style[m] = v }
  end
end

class Shoes
  VERSION = "Raisins"

  NotFound = proc do
    para "404 NOT FOUND, GUYS!"
  end
 
  ShyMake = proc do |s|
    proc do
      stack do
        background rgb(240, 240, 150)
        stack :margin => 10 do
          subtitle "ShyMaker", :margin => 0
          inscription "for Shoes #{Shoes::VERSION}"
        end
      end
      stack :margin_left => 40 do
        @done =
          stack :margin => 20, :hidden => true do
            para "Your .shy is fully baked."
          end
        @make =
          stack :margin => 20, :hidden => true do
            para "Making the Shy" 
            @make_text = para "Creating file..."
            @prog = progress
          end
        info =
          stack :margin => 10 do
            stack :margin => 10 do
              para "Application name"
              @shy_name = edit_line
            end
            stack :margin => 10 do
              para "Creator"
              @shy_creator = edit_line
            end
            stack :margin => 10 do
              para "Version"
              @shy_version = edit_line
            end
            stack :margin => 10 do
              para "Launch"
              @shy_launch = list_box :items => Shy.launchable(s)
            end
            stack :margin => 10 do
              button "Ready" do
                shy_save = ask_save_file

                info.hide
                @make.show

                shy = Shy.new
                shy.name = @shy_name.text
                shy.creator = @shy_creator.text
                shy.version = @shy_version.text
                shy.launch =  @shy_launch.text

                Thread.start do
                  Shy.c(shy_save, shy, s) do |_name, _perc, _left|
                    @make_text.replace "Adding #{_name}"
                    @prog.fraction = _perc
                  end
                  @make.hide
                  @done.show
                end
              end
            end
          end
      end
    end
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

    opts.on("-g", "--gem",
            "Passes commands to RubyGems.") do
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
    line = caller[-1]
    return if @setups[line]
    script = line[/^(.+?):/, 1]
    set = Shoes::Setup.new(script, &blk)
    @setups[line] = true
    unless set.steps.empty?
      raise SettingUp
    end
  end

  def self.show_selector
    fname = ask_open_file
    Shoes.load(fname) if fname
  end

  def self.make_shy(s)
    Shoes.app(&ShyMake.call(s))
  end

  def self.show_manual
    Shoes.app(&Shoes::Help)
  end

  def self.show_log
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
    Shoes::Setup.init
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

  def self.load(path)
    uri = 
      if path =~ SHOES_URL_RE
        URI("http://#$1:53045#$2")
      else
        URI(path) rescue nil
      end

    case uri
    when URI::HTTP
      Shoes.app do
        eval(uri.read)
      end
    else
      path = File.expand_path(path.gsub(/\\/, "/"))
      if path =~ /\.shy$/
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
      eval(File.read(path), TOPLEVEL_BINDING, path)
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
