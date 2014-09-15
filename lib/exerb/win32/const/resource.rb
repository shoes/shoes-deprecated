
#==============================================================================#
# $Id: resource.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

module Exerb::Win32::Const

  IMAGE_RESOURCE_NAME_IS_STRING    = 0x80000000
  IMAGE_RESOURCE_DATA_IS_DIRECTORY = 0x80000000

  RT_CURSOR       = 1
  RT_BITMAP       = 2
  RT_ICON         = 3
  RT_MENU         = 4
  RT_DIALOG       = 5
  RT_STRING       = 6
  RT_FONTDIR      = 7
  RT_FONT         = 8
  RT_ACCELERATOR  = 9
  RT_RCDATA       = 10
  RT_MESSAGETABLE = 11
  RT_GROUP_CURSOR = 12
  RT_GROUP_ICON   = 14
  RT_NAMETABLE    = 15
  RT_VERSION      = 16
  RT_DLGINCLUDE   = 17
  RT_PLUGPLAY     = 19
  RT_VXD          = 20
  RT_ANICURSOR    = 21
  RT_ANIICON      = 22
  RT_HTML         = 23

  VS_FFI_SIGNATURE     = 0xFEEF04BD
  VS_FFI_STRUCVERSION  = 0x00010000
  VS_FFI_FILEFLAGSMASK = 0x0000003F
  VOS_NT_WINDOWS32     = 0x00040004
  VFT_APP              = 0x00000001
  VFT_DLL              = 0x00000002
  VFT2_UNKNOWN         = 0x00000000

end # Exerb::Win32::Const

#==============================================================================#
#==============================================================================#
