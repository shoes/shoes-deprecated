
#==============================================================================#
# $Id: name_table.rb,v 1.4 2005/04/17 15:17:26 yuya Exp $
#==============================================================================#

require 'exerb/utility'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::NameTable

  def initialize
    @entries = []
  end

  def add(name)
    @entries.find { |entry|
      if entry.name == name
        return entry.id
      end
    }

    @entries << Exerb::NameTable::Entry.new(@entries.size + 1, name)

    return @entries.last.id
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

    table_header = Exerb::NameTable::Header.new
    table_header.signature         = Exerb::NameTable::Header::SIGNATURE
    table_header.number_of_headers = @entries.size
    table_header.offset_of_headers = Exerb::Utility.alignment16(table_header.pack).size
    table_header.offset_of_pool    = table_header.offset_of_headers + packed_headers.size

    return Exerb::Utility.alignment16(table_header.pack) + packed_headers + packed_pool
  end

end # Exerb::NameTable

#==============================================================================#

class Exerb::NameTable::Header

  SIGNATURE = 0x0100544E

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

end # Exerb::NameTable::Header

#==============================================================================#

class Exerb::NameTable::Entry

  def initialize(id, name)
    @id   = id
    @name = name
  end

  attr_reader :id, :name

  def pack_header(pool)
    entry_header = Exerb::NameTable::Entry::Header.new
    entry_header.id             = @id
    entry_header.offset_of_name = pool.size

    return entry_header.pack
  end

  def pack_pool
    return @name + "\0"
  end

end # Exerb::NameTable::Entry

#==============================================================================#

class Exerb::NameTable::Entry::Header

  def initialize
    @id             = 0
    @offset_of_name = 0
  end

  attr_accessor :id, :offset_of_name

  def pack
    return [@id, @offset_of_name].pack('SL')
  end

end # Exerb::NameTable::Entry::Header

#==============================================================================#
#==============================================================================#
