
#==============================================================================#
# $Id: image_section_header.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/base'

#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::ImageSectionHeader < Exerb::Win32::Struct::Base

  FORMAT = 'Z8LLLLLLSSL'

  def initialize
    @name                   = ""
    @virtual_size           = 0
    @virtual_address        = 0
    @size_of_raw_data       = 0
    @pointer_to_raw_data    = 0
    @pointer_to_relocations = 0
    @pointer_to_linenumbers = 0
    @number_of_relocations  = 0
    @number_of_linenumbers  = 0
    @characteristics        = 0
  end

  attr_accessor :name, :virtual_size, :virtual_address, :size_of_raw_data, :pointer_to_raw_data, :pointer_to_relocations, :pointer_to_linenumbers, :number_of_relocations, :number_of_linenumbers, :characteristics

  def pack
    return [@name, @virtual_size, @virtual_address, @size_of_raw_data, @pointer_to_raw_data, @pointer_to_relocations, @pointer_to_linenumbers, @number_of_relocations, @number_of_linenumbers, @characteristics].pack(FORMAT)
  end

  def unpack(bin)
    @name, @virtual_size, @virtual_address, @size_of_raw_data, @pointer_to_raw_data, @pointer_to_relocations, @pointer_to_linenumbers, @number_of_relocations, @number_of_linenumbers, @characteristics = bin.unpack(FORMAT)
    return self
  end

  alias :physical_address  :virtual_size
  alias :physical_address= :virtual_size=

end # Exerb::Win32::Struct::ImageSectionHeader

#==============================================================================#
#==============================================================================#
