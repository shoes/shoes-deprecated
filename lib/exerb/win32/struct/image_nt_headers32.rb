
#==============================================================================#
# $Id: image_nt_headers32.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/base'
require 'exerb/win32/struct/image_file_header'
require 'exerb/win32/struct/image_optional_header32'

#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::ImageNtHeaders32 < Exerb::Win32::Struct::Base

  SIGNATURE = 0x00004550

  def initialize
    @signature       = 0
    @file_header     = nil
    @optional_header = nil
  end

  attr_accessor :signature, :file_header, :optional_header

  def pack
    return [signature].pack('L') + @file_header.pack + @optional_header.pack
  end

  def unpack
    raise NotImplementedError
  end

  def read(io)
    @position        = io.pos
    @signature       = io.read(4).unpack('L')[0]
    raise('nt headers have invalid signature') unless @signature == SIGNATURE
    @file_header     = Exerb::Win32::Struct::ImageFileHeader.read(io)
    @optional_header = Exerb::Win32::Struct::ImageOptionalHeader32.read(io)
    return self
  end

end # Exerb::Win32::Struct::ImageNtHeaders32

#==============================================================================#
#==============================================================================#
