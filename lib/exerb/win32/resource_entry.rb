
#==============================================================================#
# $Id: resource_entry.rb,v 1.8 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/image_resource_data_entry'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::ResourceEntry

  def initialize(entry_data, lang = nil)
    @entry_data = Exerb::Win32::ResourceEntry::Data.new(entry_data)
    @lang       = lang
  end

  attr_accessor :entry_data, :lang

  def self.read(io, base, lang)
    return self.new(nil).read(io, base, lang)
  end

  def serialize(elements, level)
    elements[1][level] = [] if elements[1][level].nil?
    elements[1][level] << self
    @entry_data.serialize(elements, level + 1)
    return elements
  end

  def pack(table, reloc, base)
    data = Exerb::Win32::Struct::ImageResourceDataEntry.new
    data.offset_to_data = table[@entry_data] + base
    data.size_of_data   = @entry_data.size
    data.code_page      = 0x0000
    data.reserved       = 0x0000

    reloc[self] ||= []
    reloc[self] << 0

    return data.pack
  end

  def read(io, base, lang)
    data = Exerb::Win32::Struct::ImageResourceDataEntry.read(io)
    io.seek(data.offset_to_data - base)

    @entry_data = Exerb::Win32::ResourceEntry::Data.read(io, base, data.size_of_data)
    @lang       = lang

    return self
  end

end # Exerb::Win32::ResourceEntry

#==============================================================================#

class Exerb::Win32::ResourceEntry::Data

  def initialize(data)
    @data = data
  end

  attr_accessor :data

  def self.read(io, base, size)
    return self.new(nil).read(io, base, size)
  end

  def serialize(elements, level)
    elements[2][level] = [] if elements[2][level].nil?
    elements[2][level] << self
    return elements
  end

  def pack(table, reloc, base)
    return Exerb::Utility.alignment16(@data)
  end

  def size
    return @data.size
  end

  def read(io, base, size)
    @data = io.read(size)
    return self
  end

end # Exerb::Win32::ResourceEntry::Data

#==============================================================================#
#==============================================================================#
