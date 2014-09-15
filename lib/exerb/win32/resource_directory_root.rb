
#==============================================================================#
# $Id: resource_directory_root.rb,v 1.6 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/utility'
require 'exerb/win32/resource_directory'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::ResourceDirectoryRoot < Exerb::Win32::ResourceDirectory

  def initialize
    super(0)
  end

  def self.read(io, base)
    return self.new.read(io, base, 0)
  end

  def serial
    return self.serialize([[], [], []], 0)
  end

  def pack_all(base, reloc_table = [])
    table   = Hash.new(0)
    reloc   = {}
    address = 0
    buffer  = ''

    self.serial.each { |entries|
      entries.flatten!
      entries.compact!
    }.each { |entries|
      entries.each { |entry|
        table[entry] = address
        address += entry.pack(table, {}, base).size
      }
      address += (address % 16 == 0 ? 0 : 16 - address % 16)
    }.each { |entries|
      entries.each { |entry|
        buffer << entry.pack(table, reloc, base)
      }
      buffer = Exerb::Utility.alignment16(buffer)
    }

    reloc.keys.each { |obj|
      reloc[obj].each { |delta|
        reloc_table << table[obj] + delta
      }
    }
    reloc_table.sort!

    return buffer
  end

end # Exerb::Win32::ResourceDirectoryRoot

#==============================================================================#
#==============================================================================#
