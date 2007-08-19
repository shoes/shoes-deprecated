#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
require 'open-uri'
require 'shoes/shy'

class Range 
  def rand 
    conv = (self.end === Integer && self.begin === Integer ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv) 
  end 
end

class Shoes
  @mounts = []

  NoScript = proc do
    script = ask_open_file
    Shoes.load(script)
    goto "/"
  end

  NotFound = proc do
    text "404 NOT FOUND, GUYS!"
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
      [@main_app || NoScript]
    else
      [NotFound]
    end
  end

  def self.load(path)
    path = File.expand_path(path.gsub(/\\/, "/"))
    if path =~ /\.shy$/
      base = File.basename(path, ".shy")
      tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
      shy = Shy.x(path, tmpdir)
      Dir.chdir(tmpdir)
      Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
      path = shy.launch
    end
    eval(File.read(path))
  end

  def self.url(path, meth)
    Shoes.mount(path, [self, meth])
  end
end
