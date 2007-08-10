#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
require 'open-uri'

class Range 
  def rand 
    (Kernel.rand * (self.end - self.begin)) + self.begin 
  end 
end

module Shoes
  @mounts = {}

  NoScript = proc do
    script = ask_open_file
    Shoes.load(script)
    goto "/"
  end

  NotFound = proc do
    text "No script was launched.\n\nTry running: shoes samples/timer.rb"
  end
 
  def self.mount(path, &blk)
    @mounts[path] = blk
  end

  def self.run(path)
    uri = URI(path)
    if uri.path == "/"
      @main_app or NoScript
    elsif @mounts.has_key? path
      @mounts[path]
    else
      NotFound
    end
  end

  def self.load(path)
    path.gsub!(/\\/, "/")
    eval(File.read(path))
  end
end
