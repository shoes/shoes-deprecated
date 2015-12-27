
#==============================================================================#
# $Id: icon_header.rb,v 1.3 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::IconHeader < Exerb::Win32::Struct::Base

  FORMAT = 'SSS'

  def initialize
    @reserved = 0
    @type     = 0
    @count    = 0
  end

  attr_accessor :reserved, :type, :count

  def pack
    return [@reserved, @type, @count].pack(FORMAT)
  end

  def unpack(bin)
    @reserved, @type, @count = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::IconHeader

#==============================================================================#
#==============================================================================#
