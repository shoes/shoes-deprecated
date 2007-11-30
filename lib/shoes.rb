#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
ARGV.delete_if { |x| x =~ /-psn_/ }

require 'open-uri'
require 'optparse'
if Object.const_defined? :Shoes
  require 'shoes/cache'
  require 'shoes/help'
end
require 'shoes/inspect'
require 'shoes/shy'
 
class Range 
  def rand 
    conv = (self.end === Integer && self.begin === Integer ? :to_i : :to_f)
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
  VERSION = "0.1"

  NoScript = proc do
    script = ask_open_file
    Shoes.load(script)
    visit "/"
  end

  NotFound = proc do
    para "404 NOT FOUND, GUYS!"
  end
 
  ShyMake = proc do |s|
    proc do
      stack :margin => 10 do
        background rgb(240, 240, 150)
        title "ShyMaker"
        smalltitle "for Shoes #{Shoes::VERSION}"
      end
      stack do
        progress =
          stack :margin => 20 do
            para "Making the Shy"
            progress
          end
        progress.hide
        info =
          stack :margin => 10 do
            stack :margin => 10 do
              para "Application name"
              edit_line
            end
            stack :margin => 10 do
              para "Creator"
              edit_line
            end
            stack :margin => 10 do
              para "Version"
              edit_line
            end
            stack :margin => 10 do
              para "Launch"
              list_box :items => Shy.launchable(s)
            end
            stack :margin => 10 do
              button "Ready" do
                info.hide
                progress.show
              end
            end
          end
      end
    end
  end

  @mounts = []
  @main_app = NoScript

  OPTS = OptionParser.new do |opts|
    opts.banner = "Usage: shoes [options] (app.rb or app.shy)"
    
    opts.separator ""
    opts.separator "Specific options:"
    
    opts.on("-m", "--manual",
            "Open the built-in manual.") do |s|
      @main_app = Shoes::Help
    end
    opts.on("-s", "--shy DIRECTORY",
            "Compress a directory into a Shoes YAML (SHY) archive.") do |s|
      @main_app = ShyMake.call(s)
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
      [@main_app]
    when SHOES_URL_RE
      [proc { eval(URI("http://#$1:53045#$2").read) }]
    else
      [NotFound]
    end
  end

  def self.args!
    OPTS.parse! ARGV
    ARGV[0] or (!!@main_app)
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
      end
      eval(File.read(path), TOPLEVEL_BINDING)
    end
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
end
