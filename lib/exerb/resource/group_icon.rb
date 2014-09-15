
#==============================================================================#
# $Id: group_icon.rb,v 1.8 2005/05/05 02:26:29 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/icon_header'
require 'exerb/win32/struct/icon_res_entry'
require 'exerb/resource/base'

#==============================================================================#

module Exerb
  class Resource
  end # Resource
end # Exerb

#==============================================================================#

class Exerb::Resource::GroupIcon < Exerb::Resource::Base

  def initialize
    super()
    @entries = []
  end

  attr_reader :entries

  def add(id, icon)
    @entries << Entry.new(id, icon.width, icon.height, icon.bit_count, icon.size)
    return self
  end

  def pack
    header = Exerb::Win32::Struct::IconHeader.new
    header.reserved = 0
    header.type     = 1
    header.count    = @entries.size

    packed_header  = header.pack
    packed_entries = @entries.collect { |entry| entry.pack }.join

    return packed_header + packed_entries
  end

  class Entry

    def initialize(id, width, height, bit_count, size)
      @id        = id
      @widht     = width
      @height    = height
      @bit_count = bit_count
      @size      = size
    end

    attr_accessor :id, :width, :height, :bit_count, :size

    def pack
      case @bit_count
      when 4 then cc, bc = 16, 4
      when 8 then cc, bc =  0, 8
      else raise "invalid bit count -- #{@bit_count}"
      end

      icon_dir_entry = Exerb::Win32::Struct::IconResEntry.new
      icon_dir_entry.width        = @widht
      icon_dir_entry.height       = @height
      icon_dir_entry.color_count  = cc
      icon_dir_entry.reserved     = 0
      icon_dir_entry.planes       = 1
      icon_dir_entry.bit_count    = bc
      icon_dir_entry.bytes_in_res = @size
      icon_dir_entry.image_offset = @id

      return icon_dir_entry.pack
    end

  end # Entry

end # Exerb::Resource::GroupIcon

#==============================================================================#
#==============================================================================#
