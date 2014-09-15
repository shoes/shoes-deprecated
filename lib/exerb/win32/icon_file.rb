
#==============================================================================#
# $Id: icon_file.rb,v 1.4 2005/05/05 02:26:29 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/icon_header'
require 'exerb/win32/struct/icon_dir_entry'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::IconFile

  def initialize
    @entries = []
  end

  attr_accessor :entries

  def self.read(input)
    case input
    when IO     then return self.new.read(input)
    when String then return File.open(input, 'rb') { |file| self.read(file) }
    else raise(ArgumentError, "input must be IO or String object")
    end
  end

  def read(io)
    base = io.pos
    icon_header = Exerb::Win32::Struct::IconHeader.read(io)

    @entries = (1..icon_header.count).collect {
      Exerb::Win32::Struct::IconDirEntry.read(io)
    }.collect { |icon_dir_entry|
      io.seek(base + icon_dir_entry.image_offset)

      entry = Exerb::Win32::IconFile::Entry.new
      entry.width     = icon_dir_entry.width
      entry.height    = icon_dir_entry.height
      entry.bit_count = icon_dir_entry.bit_count
      entry.bit_count = 4 if entry.bit_count == 0 && icon_dir_entry.color_count == 16
      entry.bit_count = 8 if entry.bit_count == 0 && icon_dir_entry.bytes_in_res == 2216
      entry.bit_count = 8 if entry.bit_count == 0 && icon_dir_entry.bytes_in_res == 1384
      entry.value     = io.read(icon_dir_entry.bytes_in_res)
      entry
    }

    return self
  end

end # Exerb::Win32::IconFile

#==============================================================================#

class Exerb::Win32::IconFile::Entry

  def initialize
    @width     = nil
    @height    = nil
    @bit_count = nil
    @value     = nil
  end

  attr_accessor :width, :height, :bit_count, :value

end # Exerb::Win32::IconFile::Entry

#==============================================================================#
#==============================================================================#
