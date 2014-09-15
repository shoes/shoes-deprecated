
#==============================================================================#
# $Id: version_info_block.rb,v 1.5 2005/04/30 15:16:04 yuya Exp $
#==============================================================================#

require 'exerb/utility'
require 'exerb/win32/struct/base'

#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::VersionInfoBlock < Exerb::Win32::Struct::Base

  def initialize
    @total_length = 0
    @value_length = 0
    @type         = 0
    @key          = ""
    @data         = ""
  end

  attr_accessor :total_length, :value_length, :type, :key, :data

  def pack
    return Exerb::Utility.alignment(self.pack_header + self.pack_string, 4) + @data.to_s
  end

  def pack_header
    return [@total_length, @value_length, @type].pack('SSS')
  end

  def pack_string
    return @key.to_s + "\0\0"
  end
  protected :pack_string

  def unpack
    raise NotImplementedError
  end

  def read(io)
    @total_length = io.read(2).unpack('S')[0]
    @value_length = io.read(2).unpack('S')[0]
    @type         = io.read(2).unpack('S')[0]
    @key          = self.read_string(io)
    @data         = io.read(@value_length)

    return self
  end

  def read_string(io)
    str = ''

    while (ch = io.read(2)) != "\0\0"
      str << ch
    end

    io.seek((io.pos % 4 == 0 ? 0 : 4 - io.pos % 4), IO::SEEK_CUR)

    return str
  end
  protected :read_string

end # Exerb::Win32::Struct::VersionInfoBlock

#==============================================================================#
#==============================================================================#
