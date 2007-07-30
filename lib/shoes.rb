#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
require 'open-uri'

module Shoes
  @mounts = {}

  NotFound = proc do
    text "No script was launched.\n\nTry running: shoes samples/timer.rb"
  end
 
  def self.mount(path, &blk)
    @mounts[path] = blk
  end

  def self.run(path)
    uri = URI(path)
    if uri.path == "/"
      @main_app or NotFound
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
