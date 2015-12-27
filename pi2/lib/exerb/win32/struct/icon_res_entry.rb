
#==============================================================================#
# $Id: icon_res_entry.rb,v 1.3 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::IconResEntry < Exerb::Win32::Struct::Base

  FORMAT = 'CCCCSSLS'

  def initialize
    @width        = 0
    @height       = 0
    @color_count  = 0
    @reserved     = 0
    @planes       = 0
    @bit_count    = 0
    @bytes_in_res = 0
    @image_offset = 0
  end

  attr_accessor :width, :height, :color_count, :reserved, :planes, :bit_count, :bytes_in_res, :image_offset

  def pack
    return [@width, @height, @color_count, @reserved, @planes, @bit_count, @bytes_in_res, @image_offset].pack(FORMAT)
  end

  def unpack(bin)
    @width, @height, @color_count, @reserved, @planes, @bit_count, @bytes_in_res, @image_offset = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::IconResEntry

#==============================================================================#
#==============================================================================#
