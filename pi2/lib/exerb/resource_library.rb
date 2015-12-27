
#==============================================================================#
# $Id: resource_library.rb,v 1.7 2005/04/30 15:16:04 yuya Exp $
#==============================================================================#

require 'exerb/utility'
require "exerb/resource"

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::ResourceLibrary

  def initialize(rsrc)
    @rsrc = rsrc
  end

  attr_accessor :rsrc

  def self.create
    return self.new(Exerb::Resource.new)
  end

  def pack
    rsrc = @rsrc.pack(0x00001000)
    stab = Exerb::ResourceLibrary::Stab.new

    size_of_rsrc_with_padding = Exerb::Utility.alignment4k(rsrc).size
    size_of_stab_with_padding = Exerb::Utility.alignment4k(stab.pack).size

    stab.time_date_stamp              = Time.now.to_i
    stab.size_of_initialized_data     = size_of_rsrc_with_padding
    stab.size_of_image                = size_of_stab_with_padding + size_of_rsrc_with_padding
    stab.rsrc_section_virtual_address = size_of_stab_with_padding
    stab.rsrc_section_virtual_size    = rsrc.size
    stab.rsrc_section_raw_size        = size_of_rsrc_with_padding

    return Exerb::Utility.alignment4k(stab.pack) + Exerb::Utility.alignment4k(rsrc)
  end

  def write(out)
    case out
    when IO     then out.write(self.pack)
    when String then File.open(out, 'wb') { |file| self.write(file) }
    else raise(ArgumentError, 'arg must be IO or String object')
    end
    return nil
  end

end # Exerb::ResourceLibrary

#==============================================================================#

class Exerb::ResourceLibrary::Stab

  BASE_BINARY =
    "\x4D\x5A\x90\x00\x03\x00\x00\x00\x04\x00\x00\x00\xFF\xFF\x00\x00" +
    "\xB8\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xB0\x00\x00\x00" +
    "\x0E\x1F\xBA\x0E\x00\xB4\x09\xCD\x21\xB8\x01\x4C\xCD\x21\x54\x68" +
    "\x69\x73\x20\x70\x72\x6F\x67\x72\x61\x6D\x20\x63\x61\x6E\x6E\x6F" +
    "\x74\x20\x62\x65\x20\x72\x75\x6E\x20\x69\x6E\x20\x44\x4F\x53\x20" +
    "\x6D\x6F\x64\x65\x2E\x0D\x0D\x0A\x24\x00\x00\x00\x00\x00\x00\x00" +
    "\xEB\x20\x35\xDB\xAF\x41\x5B\x88\xAF\x41\x5B\x88\xAF\x41\x5B\x88" +
    "\x68\x47\x5D\x88\xAE\x41\x5B\x88\x52\x69\x63\x68\xAF\x41\x5B\x88" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x50\x45\x00\x00\x4C\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\xE0\x00\x0F\x21\x0B\x01\x06\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00" +
    "\x00\x10\x00\x00\x00\x00\x00\x10\x00\x10\x00\x00\x00\x10\x00\x00" +
    "\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00" +
    "\x00\x00\x10\x00\x00\x10\x00\x00\x00\x00\x10\x00\x00\x10\x00\x00" +
    "\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x2E\x72\x73\x72\x63\x00\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00" +
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x40"

  def initialize
    @time_date_stamp              = 0
    @size_of_initialized_data     = 0
    @size_of_image                = 0
    @rsrc_section_virtual_address = 0
    @rsrc_section_virtual_size    = 0
    @rsrc_section_raw_size        = 0
  end

  attr_accessor :time_date_stamp, :size_of_initialized_data, :size_of_image, :rsrc_section_virtual_address, :rsrc_section_virtual_size, :rsrc_section_raw_size

  def pack
    binary = BASE_BINARY.dup

    binary[0x00B8, 4] = [@time_date_stamp].pack('L')
    binary[0x00D0, 4] = [@size_of_initialized_data].pack('L')
    binary[0x0100, 4] = [@size_of_image].pack('L')
    binary[0x0138, 4] = [@rsrc_section_virtual_address].pack('L')
    binary[0x013C, 4] = [@rsrc_section_virtual_size].pack('L')
    binary[0x01B0, 4] = [@rsrc_section_virtual_size].pack('L')
    binary[0x01B4, 4] = [@rsrc_section_virtual_address].pack('L')
    binary[0x01B8, 4] = [@rsrc_section_raw_size].pack('L')

    return binary
  end

end # Exerb::ResourceLibrary::Stab

#==============================================================================#
#==============================================================================#
