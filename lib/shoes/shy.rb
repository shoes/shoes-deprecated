#
# lib/shoes/shy.rb
# Shy, the Shoes YAML archive format
#
require 'digest/md5'
require 'zlib'
require 'shoes/minitar'
require 'find'
require 'tmpdir'
require 'yaml'

class Shoes
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
end

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
    File.open(path, "wb") do |f|
      f << [MAGIC, VERSION, meta.length].pack(LAYOUT)
      f << meta
      self.czf(f, d, &blk)
    end
  end

  def self.progress(total, &blk)
    if blk
      last, left = 0.0, total
      proc do |action, name, stats|
        if action == :file_progress
          left -= stats[:currinc]
          prg = 1.0 - (left.to_f / total.to_f)
          blk[name, (last = prg), left] if prg - last > 0.02
        end
      end
    end
  end

  def self.czf(f, d, &blk)
    total = du(d)
    Dir.chdir(d) do
      out = Zlib::GzipWriter.new(f)
      files = ["."]

      Archive::Tar::Minitar.pack(files, out, &blk)
    end
  end

  def self.xzf(f, d, &blk)
    gz = Zlib::GzipReader.new(f)
    Archive::Tar::Minitar.unpack(gz, d, &blk)
  end

  def self.md5sum(path)
    digest = Digest::MD5.new
    File.open(path, "rb") do |f|
      digest.update f.read(8192) until f.eof
    end
    digest.hexdigest
  end

  def self.hrun(f)
    b, i = 0, 1
    last = 65535
    while i < last
      case f.readline
      when /OLDSKIP=(\d+)/
        last = $1.to_i 
      when /FULLSIZE=(\d+)/
        b = $1.to_i
      end
      i += 1
    end
    b
  end
end
