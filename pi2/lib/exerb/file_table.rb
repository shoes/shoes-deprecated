
#==============================================================================#
# $Id: file_table.rb,v 1.15 2007/02/26 10:20:44 yuya Exp $
#==============================================================================#

require 'exerb/utility'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::FileTable

  def initialize
    @entries  = []
  end

  def add(id, data, flag)
    @entries << Exerb::FileTable::Entry.new(id, data, flag)
    return @entries.last
  end

  def add_from_file(id, path, flag)
    return File.open(path, 'rb') { |file| self.add(id, file.read, flag) }
  end

  def add_ruby_script(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_RUBY_SCRIPT | flag)
  end

  def add_ruby_script_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_RUBY_SCRIPT | flag)
  end

  def add_extension_library(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_EXTENSION_LIBRARY | flag)
  end

  def add_extension_library_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_EXTENSION_LIBRARY | flag)
  end

  def add_dynamic_library(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_DYNAMIC_LIBRARY | flag)
  end

  def add_dynamic_library_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_DYNAMIC_LIBRARY | flag)
  end

  def add_resource_library(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_RESOURCE_LIBRARY | flag)
  end

  def add_resource_library_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_RESOURCE_LIBRARY | flag)
  end

  def add_data_binary(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_DATA_BINARY | flag)
  end

  def add_data_binary_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_DATA_BINARY | flag)
  end

  def add_compiled_script(id, data, flag = 0)
    return self.add(id, data, Entry::FLAG_TYPE_COMPILED_SCRIPT | flag)
  end

  def add_compiled_script_from_file(id, path, flag = 0)
    return self.add_from_file(id, path, Entry::FLAG_TYPE_COMPILED_SCRIPT | flag)
  end

  def pack
    headers = ''
    pool    = ''

    @entries.each { |entry|
      headers << entry.pack_header(pool)
      pool    << entry.pack_pool
    }

    packed_headers = Exerb::Utility.alignment16(headers)
    packed_pool    = Exerb::Utility.alignment16(pool)

    table_header = Exerb::FileTable::Header.new
    table_header.signature         = Exerb::FileTable::Header::SIGNATURE
    table_header.number_of_headers = @entries.size
    table_header.offset_of_headers = Exerb::Utility.alignment16(table_header.pack).size
    table_header.offset_of_pool    = table_header.offset_of_headers + packed_headers.size

    return Exerb::Utility.alignment16(table_header.pack) + packed_headers + packed_pool
  end

end # Exerb::FileTabel

#==============================================================================#

class Exerb::FileTable::Header

  SIGNATURE = 0x04005446

  def initialize
    @signature         = 0
    @number_of_headers = 0
    @offset_of_headers = 0
    @offset_of_pool    = 0
  end

  attr_accessor :signature, :number_of_headers, :offset_of_headers, :offset_of_pool

  def pack
    return [@signature, @number_of_headers, @offset_of_headers, @offset_of_pool].pack('LSLL')
  end

end # Exerb::FileTable::Header

#==============================================================================#

class Exerb::FileTable::Entry

  FLAG_TYPE_RUBY_SCRIPT       = 0x01
  FLAG_TYPE_EXTENSION_LIBRARY = 0x02
  FLAG_TYPE_DYNAMIC_LIBRARY   = 0x03
  FLAG_TYPE_RESOURCE_LIBRARY  = 0x04
  FLAG_TYPE_DATA_BINARY       = 0x05
  FLAG_TYPE_COMPILED_SCRIPT   = 0x06
  FLAG_NO_REPLACE_FUNCTION    = 0x08

  def initialize(id, data, flag)
    @id   = id
    @data = data
    @flag = flag
  end

  attr_reader :id, :data, :flag

  def pack_header(pool)
    entry_header = Exerb::FileTable::Entry::Header.new
    entry_header.id             = @id
    entry_header.offset_of_file = pool.size
    entry_header.size_of_file   = @data.size
    entry_header.flag_of_file   = @flag

    return entry_header.pack
  end

  def pack_pool
    return Exerb::Utility.alignment16(@data)
  end

end # Exerb::FileTable::Entry

#==============================================================================#

class Exerb::FileTable::Entry::Header

  def initialize
    @id             = 0
    @offset_of_file = 0
    @size_of_file   = 0
    @flag_of_file   = 0
  end

  attr_accessor :id, :offset_of_file, :size_of_file, :flag_of_file

  def pack
    return [@id, @offset_of_file, @size_of_file, @flag_of_file].pack('SLLC')
  end

end # Exerb::FileTable::Entry::Header

#==============================================================================#
#==============================================================================#
