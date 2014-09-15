
#==============================================================================#
# $Id: icon.rb,v 1.6 2005/05/05 02:26:29 yuya Exp $
#==============================================================================#

require 'exerb/error'
require 'exerb/win32/icon_file'
require 'exerb/resource/base'

#==============================================================================#

module Exerb
  class Resource
  end # Resource
end # Exerb

#==============================================================================#

class Exerb::Resource::Icon < Exerb::Resource::Base

  def initialize
    super()
    @width     = nil
    @height    = nil
    @bit_count = nil
    @value     = nil
  end

  attr_accessor :width, :height, :bit_count, :value

  def self.read(input, width, height, bit_count)
    icon_file  = Exerb::Win32::IconFile.read(input)
    icon_entry = icon_file.entries.find { |entry| entry.width == width && entry.height == height && entry.bit_count == bit_count }
    raise(Exerb::ExerbError, "no such icon #{width}x#{height}x#{bit_count} in #{input}") unless icon_entry

    icon = self.new
    icon.width     = icon_entry.width
    icon.height    = icon_entry.height
    icon.bit_count = icon_entry.bit_count
    icon.value     = icon_entry.value

    return icon
  end

  def pack
    return @value
  end

end # Exerb::Resource::Icon

#==============================================================================#
#==============================================================================#
