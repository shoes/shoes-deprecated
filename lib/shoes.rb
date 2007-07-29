#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
require 'open-uri'

class Canvas
  def text(str)
    prep = str[/\n*( +)/, 1]
    str = str.strip.gsub(/^#{prep}/, '')
    html = str.gsub(/\r?\n/, '')
    markup("<span color='black'>#{html}</span>")
  end
end

module Shoes
  @mounts = {}

  NotFound = proc do
    markup "<span color='black'>Not Found</span>"
  end
 
  def self.escape(string)
   string.gsub(/&/n, '&amp;').gsub(/\"/n, '&quot;').gsub(/>/n, '&gt;').gsub(/</n, '&lt;')
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
