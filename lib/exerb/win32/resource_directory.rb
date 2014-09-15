
#==============================================================================#
# $Id: resource_directory.rb,v 1.7 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/image_resource_directory'
require 'exerb/win32/struct/image_resource_directory_entry'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::ResourceDirectory

  def initialize(name)
    @name    = name
    @entries = []

    yield(self) if block_given?
  end

  attr_accessor :name, :entries

  def self.read(io, base, name = nil)
    return self.new(nil).read(io, base, name)
  end

  def <<(entry)
    @entries << entry
  end

  def serialize(elements, level)
    elements[0][level] = [] if elements[0][level].nil?
    elements[0][level] << self
    @entries.each { |entry| elements[0][level] << Exerb::Win32::ResourceDirectory::Entry.new(entry) }
    @entries.each { |entry| entry.serialize(elements, level + 1) }
    return elements
  end

  def pack(table, reloc, base)
    dir = Exerb::Win32::Struct::ImageResourceDirectory.new
    dir.characteristics        = 0
    dir.time_date_stamp        = 0
    dir.major_version          = 0
    dir.minor_version          = 0
    dir.number_of_name_entries = 0
    dir.number_of_id_entries   = @entries.size

    return dir.pack
  end

  def read(io, base, name)
    dir = Exerb::Win32::Struct::ImageResourceDirectory.read(io)

    @name    = name
    @entries = (1..(dir.number_of_name_entries + dir.number_of_id_entries)).collect {
      Exerb::Win32::Struct::ImageResourceDirectoryEntry.read(io)
    }.collect { |entry|
      if entry.data_is_directory?
        io.seek(entry.offset_to_directory)
        Exerb::Win32::ResourceDirectory.read(io, base, entry.name)
      else
        io.seek(entry.offset_to_data)
        Exerb::Win32::ResourceEntry.read(io, base, entry.name)
      end
    }

    return self
  end

end # Exerb::Win32::ResourceDirectory

#==============================================================================#

class Exerb::Win32::ResourceDirectory::Entry

  def initialize(entry)
    @entry = entry
  end

  attr_accessor :entry

  def pack(table, reloc, base)
    entry = Exerb::Win32::Struct::ImageResourceDirectoryEntry.new

    if @entry.kind_of?(Exerb::Win32::ResourceDirectory)
      entry.name                = @entry.name
      entry.offset_to_directory = table[@entry]
    else
      entry.name                = @entry.lang
      entry.offset_to_data      = table[@entry]
    end

    return entry.pack
  end

end # Exerb::Win32::ResourceDirectory::Entry

#==============================================================================#
#==============================================================================#
