
#==============================================================================#
# $Id: image_resource_directory_entry.rb,v 1.5 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'exerb/win32/struct/base'
require 'exerb/win32/const/resource'

#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::ImageResourceDirectoryEntry < Exerb::Win32::Struct::Base

  FORMAT = 'LL'

  def initialize
    super()
    @name           = 0
    @offset_to_data = 0
  end

  attr_accessor :name, :offset_to_data

  def pack
    return [@name, @offset_to_data].pack(FORMAT)
  end

  def unpack(bin)
    @name, @offset_to_data = bin.unpack(FORMAT)
    return self
  end

  def id
    return @name & 0xFFFF
  end

  def name_is_string?
    return (@name & Exerb::Win32::Const::IMAGE_RESOURCE_NAME_IS_STRING > 0 ? true : false)
  end

  def offset_to_string
    return @name & ~Exerb::Win32::Const::IMAGE_RESOURCE_NAME_IS_STRING
  end

  def offset_to_string=(value)
    @name = value | Exerb::Win32::Const::IMAGE_RESOURCE_NAME_IS_STRING
  end

  def data_is_directory?
    return (@offset_to_data & Exerb::Win32::Const::IMAGE_RESOURCE_DATA_IS_DIRECTORY > 0 ? true : false)
  end

  def offset_to_directory
    return @offset_to_data & ~Exerb::Win32::Const::IMAGE_RESOURCE_DATA_IS_DIRECTORY
  end

  def offset_to_directory=(value)
    @offset_to_data = value | Exerb::Win32::Const::IMAGE_RESOURCE_DATA_IS_DIRECTORY
  end

end # Exerb::Win32::Struct::ImageResourceDirectoryEntry

#==============================================================================#
#==============================================================================#
