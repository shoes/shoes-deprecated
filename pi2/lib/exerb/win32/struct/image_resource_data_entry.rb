
#==============================================================================#
# $Id: image_resource_data_entry.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::ImageResourceDataEntry < Exerb::Win32::Struct::Base

  FORMAT = 'LLLL'

  def initialize
    super()
    @offset_to_data = 0
    @size_of_data   = 0
    @code_page      = 0
    @reserved       = 0
  end

  attr_accessor :offset_to_data, :size_of_data, :code_page, :reserved

  def pack
    return [@offset_to_data, @size_of_data, @code_page, @reserved].pack(FORMAT)
  end

  def unpack(bin)
    @offset_to_data, @size_of_data, @code_page, @reserved = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::ImageResourceDataEntry

#==============================================================================#
#==============================================================================#
