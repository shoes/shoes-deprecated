#
# lib/shoes/shy.rb
# Shy, the Shoes YAML archive format
#
require 'zlib'
require 'shoes/minitar'
require 'find'
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
      path.gsub(%r!#{Regexp::quote(d)}/!, '')
    end
  end

  def self.__hdr__(f)
    hdr = f.read(10).unpack(LAYOUT)
    raise IOError, "Invalid header" if hdr[0] != MAGIC and hdr[1] > VERSION
    YAML.load(f.read(hdr[2]))
  end


  def self.du(root)
    size = 0
    Find.find(root) do |path|
      if FileTest.directory?(path)
        if File.basename(path)[0] == ?.
          Find.prune
        else
          next
        end
      else
        size += FileTest.size(path)
      end
    end
    size
  end

  def self.meta(path, d = ".")
    File.open(path, 'rb') do |f|
      shy = __hdr__(f)
    end
  end

  def self.x(path, d = ".")
    File.open(path, 'rb') do |f|
      shy = __hdr__(f)
      inp = Zlib::GzipReader.new(f)
      Archive::Tar::Minitar.unpack(inp, d)
      shy
    end
  end

  def self.c(path, shy, d, &blk)
    path = File.expand_path(path)
    meta = shy.to_yaml
    total = left = du(d)
    Dir.chdir(d) do
      File.open(path, "wb") do |f|
        f << [MAGIC, VERSION, meta.length].pack(LAYOUT)
        f << meta
        out = Zlib::GzipWriter.new(f)
        files = ["."]

        tarblk = nil
        if blk
          tarblk = proc do |action, name, stats|
            if action == :file_progress
              left -= stats[:currinc]
              blk[name, 1.0 - (left.to_f / total.to_f), left]
            end
          end
        end
        Archive::Tar::Minitar.pack(files, out, &tarblk)
      end
    end
  end
end
