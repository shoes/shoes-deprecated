#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
ARGV.delete_if { |x| x =~ /-psn_/ }

require 'open-uri'
require 'optparse'
require 'shoes/shy'
 
class Range 
  def rand 
    conv = (self.end === Integer && self.begin === Integer ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv) 
  end 
end

class Shoes
  VERSION = "0.1"

  NoScript = proc do
    script = ask_open_file
    Shoes.load(script)
    goto "/"
  end

  NotFound = proc do
    text "404 NOT FOUND, GUYS!"
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
            text "Making the Shy"
            progress
          end
        progress.hide
        info =
          stack :margin => 10 do
            stack :margin => 10 do
              text "Application name"
              edit_line
            end
            stack :margin => 10 do
              text "Creator"
              edit_line
            end
            stack :margin => 10 do
              text "Version"
              edit_line
            end
            stack :margin => 10 do
              text "Launch"
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
    
    opts.on("-s", "--shy DIRECTORY",
            "Compress a directory into a Shoes YAML (SHY) archive.") do |s|
      @main_app = ShyMake.call(s)
    end
  end

  def self.mount(path, meth, &blk)
    @mounts << [path, meth || blk]
  end

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
    if uri.path == "/"
      [@main_app]
    else
      [NotFound]
    end
  end

  def self.args!
    OPTS.parse! ARGV
    ARGV[0] or (!!@main_app)
  end

  def self.load(path)
    uri = URI(path) rescue nil
    case uri
    when URI::HTTP
      eval(uri.read, TOPLEVEL_BINDING)
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
end

class Canvas
  def title str
    text "<span font_desc='34px'>#{str}</span>"
  end
  def subtitle str
    text "<span font_desc='26px'>#{str}</span>"
  end
  def smalltitle str
    text "<span font_desc='18px'>#{str}</span>"
  end
end
