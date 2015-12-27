
#==============================================================================#
# $Id: version_info.rb,v 1.15 2006/06/02 00:47:54 yuya Exp $
#==============================================================================#

require 'exerb/utility'
require 'exerb/resource/base'
require 'exerb/win32/const/resource'
require 'exerb/win32/struct/vs_fixed_file_info'
require 'exerb/win32/struct/version_info_block'

#==============================================================================#

module Exerb
  class Resource
  end # Resource
end # Exerb

#==============================================================================#

class Exerb::Resource::VersionInfo < Exerb::Resource::Base

  def initialize
    @file_version_number    = 0
    @product_version_number = 0
    @comments               = ""
    @company_name           = ""
    @legal_copyright        = ""
    @legal_trademarks       = ""
    @file_version           = ""
    @product_version        = ""
    @product_name           = ""
    @file_description       = ""
    @internal_name          = ""
    @original_filename      = ""
    @private_build          = ""
    @special_build          = ""

    @is_dll = false
  end

  attr_accessor :file_version_number, :product_version_number, :comments, :company_name, :legal_copyright, :legal_trademarks, :file_version, :product_version, :product_name, :file_description, :internal_name, :original_filename, :private_build, :special_build, :is_dll

  def self.make_version(n1, n2, n3, n4)
    version  = (n1 & 0xFFFF) << 48
    version |= (n2 & 0xFFFF) << 32
    version |= (n3 & 0xFFFF) << 16
    version |= (n4 & 0xFFFF)

    return version
  end

  def pack
    vsffi = Exerb::Win32::Struct::VsFixedFileInfo.new
    vsffi.signature          = Exerb::Win32::Const::VS_FFI_SIGNATURE
    vsffi.struct_version     = Exerb::Win32::Const::VS_FFI_STRUCVERSION
    vsffi.file_version_ms    = @file_version_number >> 32
    vsffi.file_version_ls    = @file_version_number & 0xFFFFFFFF
    vsffi.product_version_ms = @product_version_number >> 32
    vsffi.product_version_ls = @product_version_number & 0xFFFFFFFF
    vsffi.file_flags_mask    = Exerb::Win32::Const::VS_FFI_FILEFLAGSMASK
    vsffi.file_flags         = 0x00000000
    vsffi.file_os            = Exerb::Win32::Const::VOS_NT_WINDOWS32
    vsffi.file_type          = (@is_dll ? Exerb::Win32::Const::VFT_DLL : Exerb::Win32::Const::VFT_APP)
    vsffi.file_subtype       = Exerb::Win32::Const::VFT2_UNKNOWN
    vsffi.file_date_ms       = 0x00000000
    vsffi.file_date_ls       = 0x00000000

    block = BlockType0.new('VS_VERSION_INFO', vsffi.pack) { |vvi|
      vvi << BlockType1.new('StringFileInfo') { |sfi|
        # Language:Neutral CodePage:Unicode
        sfi << BlockType1.new('000004b0') { |neutral|
          neutral << BlockType1String.new('Comments',         @comments)
          neutral << BlockType1String.new('CompanyName',      @company_name)
          neutral << BlockType1String.new('LegalCopyright',   @legal_copyright)
          neutral << BlockType1String.new('LegalTrademarks',  @legal_trademarks)
          neutral << BlockType1String.new('FileVersion',      @file_version)
          neutral << BlockType1String.new('ProductVersion',   @product_version)
          neutral << BlockType1String.new('ProductName',      @product_name)
          neutral << BlockType1String.new('FileDescription',  @file_description)
          neutral << BlockType1String.new('InternalName',     @internal_name)
          neutral << BlockType1String.new('OriginalFilename', @original_filename)
          neutral << BlockType1String.new('PrivateBuild',     @private_build)
          neutral << BlockType1String.new('SpecialBuild',     @special_build)
        }
      }
      vvi << BlockType1.new('VarFileInfo') { |vfi|
        vfi << BlockType0.new('Translation', [0x04B00000].pack('L'))
      }
    }

    return block.pack
  end

  class BlockBase

    def initialize(type, key, value = '', unicode = false)
      @type     = type
      @key      = key
      @value    = value
      @unicode  = unicode
      @children = []

      yield(self) if block_given?
    end

    attr_accessor :type, :key, :value, :unicode, :children

    def <<(child)
      @children << child
    end

    def pack
      packed_children = @children.collect { |child| child.pack }.join

      block = Exerb::Win32::Struct::VersionInfoBlock.new
      block.type         = @type
      block.key          = @key.unpack("U*").pack("v*") # UTF-8 to UTF-16LE
      block.data         = @value
      block.value_length = (@unicode ? @value.size / 2 : @value.size)
      block.total_length = block.pack.size + packed_children.size

      return Exerb::Utility.alignment(block.pack, 4) + packed_children
    end

  end # Block

  class BlockType0 < BlockBase

    def initialize(key, value = '', unicode = false)
      super(0x0000, key, value, unicode)
    end

  end # BlockType0

  class BlockType1 < BlockBase

    def initialize(key, value = '', unicode = false)
      super(0x0001, key, value, unicode)
    end

  end # BlockType1

  class BlockType1String < BlockType1

    def initialize(key, value)
      super(key, value.to_s.unpack("U*").pack("v*") + "\0\0", true) # UTF-8 to UTF-16LE
    end

  end # BlockType1String

end # Exerb::Resource::VersionInfo

#==============================================================================#
#==============================================================================#
