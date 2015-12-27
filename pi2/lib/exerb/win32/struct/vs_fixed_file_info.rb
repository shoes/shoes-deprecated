
#==============================================================================#
# $Id: vs_fixed_file_info.rb,v 1.3 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::VsFixedFileInfo < Exerb::Win32::Struct::Base

  FORMAT = 'LLLLLLLLLLLLL'

  def initialize
    super()
    @signature          = 0
    @struct_version     = 0
    @file_version_ms    = 0
    @file_version_ls    = 0
    @product_version_ms = 0
    @product_version_ls = 0
    @file_flags_mask    = 0
    @file_flags         = 0
    @file_os            = 0
    @file_type          = 0
    @file_subtype       = 0
    @file_date_ms       = 0
    @file_date_ls       = 0
  end

  attr_accessor :signature, :struct_version, :file_version_ms, :file_version_ls, :product_version_ms, :product_version_ls, :file_flags_mask, :file_flags, :file_os, :file_type, :file_subtype, :file_date_ms, :file_date_ls

  def pack
    return [@signature, @struct_version, @file_version_ms, @file_version_ls, @product_version_ms, @product_version_ls, @file_flags_mask, @file_flags, @file_os, @file_type, @file_subtype, @file_date_ms, @file_date_ls].pack(FORMAT)
  end

  def unpack(bin)
    @signature, @struct_version, @file_version_ms, @file_version_ls, @product_version_ms, @product_version_ls, @file_flags_mask, @file_flags, @file_os, @file_type, @file_subtype, @file_date_ms, @file_date_ls = bin.unpack(FORMAT)
    return self
  end

  def file_version
    return format('%x.%x.%x.%x', @file_version_ms >> 16, @file_version_ms & 0x0000FFFF, @file_version_ls >> 16, @file_version_ls & 0x0000FFFF)
  end

  def product_version
    return format('%x.%x.%x.%x', @product_version_ms >> 16, @product_version_ms & 0x0000FFFF, @product_version_ls >> 16, @product_version_ls & 0x0000FFFF)
  end

end # Exerb::Win32::Struct::VsFixedFileInfo

#==============================================================================#
#==============================================================================#
