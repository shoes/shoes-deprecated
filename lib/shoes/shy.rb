#
# lib/shoes/shy.rb
# Shy, the Shoes YAML archive format
#
require 'zlib'
require 'shoes/minitar'
require 'tmpdir'
require 'yaml'

class Shy
  VERSION = 0x0001
  MAGIC   = "_shy".freeze
  LAYOUT  = "A4SL".freeze

  yaml_as 'tag:hackety.org,2007:shy'
  attr_accessor :name, :creator, :version, :launch

  def self.launchable(d)
    Dir["#{d}/**/*.rb"].map do |path|
      path.gsub(%r!#{Regexp::quote(path)}/!, '')
    end
  end

  def self.x(path, d = ".")
    File.open(path, 'rb') do |f|
      hdr = f.read(10).unpack(LAYOUT)
      raise IOError, "Invalid header" if hdr[0] != MAGIC and hdr[1] > VERSION
      shy = YAML.load(f.read(hdr[2]))
      inp = Zlib::GzipReader.new(f)
      Archive::Tar::Minitar.unpack(inp, d)
      shy
    end
  end

  def self.c(path, shy, d)
    path = File.expand_path(path)
    meta = shy.to_yaml
    Dir.chdir(d) do
      File.open(path, "wb") do |f|
        f << [MAGIC, VERSION, meta.length].pack(LAYOUT)
        f << meta
        out = Zlib::GzipWriter.new(f)
        files = ["."]
        Archive::Tar::Minitar.pack(files, out)
      end
    end
  end
end
