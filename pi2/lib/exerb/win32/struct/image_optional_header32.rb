
#==============================================================================#
# $Id: image_optional_header32.rb,v 1.6 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::ImageOptionalHeader32 < Exerb::Win32::Struct::Base

  FORMAT = 'SCCLLLLLLLLLSSSSSSLLLLSSLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL'

  def initialize
    @magic                            = 0
    @major_linker_version             = 0
    @minor_linker_version             = 0
    @size_of_code                     = 0
    @size_of_initialized_data         = 0
    @size_of_uninitialized_data       = 0
    @address_of_entry_point           = 0
    @base_of_code                     = 0
    @base_of_data                     = 0
    @image_base                       = 0
    @section_alignment                = 0
    @file_alignment                   = 0
    @major_operating_system_version   = 0
    @minor_operating_system_version   = 0
    @major_image_version              = 0
    @minor_image_version              = 0
    @major_subsystem_version          = 0
    @minor_subsystem_version          = 0
    @win32_version_value              = 0
    @size_of_image                    = 0
    @size_of_headers                  = 0
    @checksum                         = 0
    @subsystem                        = 0
    @dll_characteristics              = 0
    @size_of_stack_reserve            = 0
    @size_of_stack_commit             = 0
    @size_of_heap_reserve             = 0
    @size_of_heap_commit              = 0
    @loader_flags                     = 0
    @number_of_rva_and_sizes          = 0
    @data_directory0_virtual_address  = 0
    @data_directory0_virtual_size     = 0
    @data_directory1_virtual_address  = 0
    @data_directory1_virtual_size     = 0
    @data_directory2_virtual_address  = 0
    @data_directory2_virtual_size     = 0
    @data_directory3_virtual_address  = 0
    @data_directory3_virtual_size     = 0
    @data_directory4_virtual_address  = 0
    @data_directory4_virtual_size     = 0
    @data_directory5_virtual_address  = 0
    @data_directory5_virtual_size     = 0
    @data_directory6_virtual_address  = 0
    @data_directory6_virtual_size     = 0
    @data_directory7_virtual_address  = 0
    @data_directory7_virtual_size     = 0
    @data_directory8_virtual_address  = 0
    @data_directory8_virtual_size     = 0
    @data_directory9_virtual_address  = 0
    @data_directory9_virtual_size     = 0
    @data_directory10_virtual_address = 0
    @data_directory10_virtual_size    = 0
    @data_directory11_virtual_address = 0
    @data_directory11_virtual_size    = 0
    @data_directory12_virtual_address = 0
    @data_directory12_virtual_size    = 0
    @data_directory13_virtual_address = 0
    @data_directory13_virtual_size    = 0
    @data_directory14_virtual_address = 0
    @data_directory14_virtual_size    = 0
    @data_directory15_virtual_address = 0
    @data_directory15_virtual_size    = 0
  end

  attr_accessor :magic, :major_linker_version, :minor_linker_version, :size_of_code, :size_of_initialized_data, :size_of_uninitialized_data, :address_of_entry_point, :base_of_code, :base_of_data, :image_base, :section_alignment, :file_alignment, :major_operating_system_version, :minor_operating_system_version, :major_image_version, :minor_image_version, :major_subsystem_version, :minor_subsystem_version, :win32_version_value, :size_of_image, :size_of_headers, :checksum, :subsystem, :dll_characteristics, :size_of_stack_reserve, :size_of_stack_commit, :size_of_heap_reserve, :size_of_heap_commit, :loader_flags, :number_of_rva_and_sizes, :data_directory0_virtual_address, :data_directory0_virtual_size, :data_directory1_virtual_address, :data_directory1_virtual_size, :data_directory2_virtual_address, :data_directory2_virtual_size, :data_directory3_virtual_address, :data_directory3_virtual_size, :data_directory4_virtual_address, :data_directory4_virtual_size, :data_directory5_virtual_address, :data_directory5_virtual_size, :data_directory6_virtual_address, :data_directory6_virtual_size, :data_directory7_virtual_address, :data_directory7_virtual_size, :data_directory8_virtual_address, :data_directory8_virtual_size, :data_directory9_virtual_address, :data_directory9_virtual_size, :data_directory10_virtual_address, :data_directory10_virtual_size, :data_directory11_virtual_address, :data_directory11_virtual_size, :data_directory12_virtual_address, :data_directory12_virtual_size, :data_directory13_virtual_address, :data_directory13_virtual_size, :data_directory14_virtual_address, :data_directory14_virtual_size, :data_directory15_virtual_address, :data_directory15_virtual_size

  def pack
    return [@magic, @major_linker_version, @minor_linker_version, @size_of_code, @size_of_initialized_data, @size_of_uninitialized_data, @address_of_entry_point, @base_of_code, @base_of_data, @image_base, @section_alignment, @file_alignment, @major_operating_system_version, @minor_operating_system_version, @major_image_version, @minor_image_version, @major_subsystem_version, @minor_subsystem_version, @win32_version_value, @size_of_image, @size_of_headers, @checksum, @subsystem, @dll_characteristics, @size_of_stack_reserve, @size_of_stack_commit, @size_of_heap_reserve, @size_of_heap_commit, @loader_flags, @number_of_rva_and_sizes, @data_directory0_virtual_address, @data_directory0_virtual_size, @data_directory1_virtual_address, @data_directory1_virtual_size, @data_directory2_virtual_address, @data_directory2_virtual_size, @data_directory3_virtual_address, @data_directory3_virtual_size, @data_directory4_virtual_address, @data_directory4_virtual_size, @data_directory5_virtual_address, @data_directory5_virtual_size, @data_directory6_virtual_address, @data_directory6_virtual_size, @data_directory7_virtual_address, @data_directory7_virtual_size, @data_directory8_virtual_address, @data_directory8_virtual_size, @data_directory9_virtual_address, @data_directory9_virtual_size, @data_directory10_virtual_address, @data_directory10_virtual_size, @data_directory11_virtual_address, @data_directory11_virtual_size, @data_directory12_virtual_address, @data_directory12_virtual_size, @data_directory13_virtual_address, @data_directory13_virtual_size, @data_directory14_virtual_address, @data_directory14_virtual_size, @data_directory15_virtual_address, @data_directory15_virtual_size].pack(FORMAT)
  end

  def unpack(bin)
    @magic, @major_linker_version, @minor_linker_version, @size_of_code, @size_of_initialized_data, @size_of_uninitialized_data, @address_of_entry_point, @base_of_code, @base_of_data, @image_base, @section_alignment, @file_alignment, @major_operating_system_version, @minor_operating_system_version, @major_image_version, @minor_image_version, @major_subsystem_version, @minor_subsystem_version, @win32_version_value, @size_of_image, @size_of_headers, @checksum, @subsystem, @dll_characteristics, @size_of_stack_reserve, @size_of_stack_commit, @size_of_heap_reserve, @size_of_heap_commit, @loader_flags, @number_of_rva_and_sizes, @data_directory0_virtual_address, @data_directory0_virtual_size, @data_directory1_virtual_address, @data_directory1_virtual_size, @data_directory2_virtual_address, @data_directory2_virtual_size, @data_directory3_virtual_address, @data_directory3_virtual_size, @data_directory4_virtual_address, @data_directory4_virtual_size, @data_directory5_virtual_address, @data_directory5_virtual_size, @data_directory6_virtual_address, @data_directory6_virtual_size, @data_directory7_virtual_address, @data_directory7_virtual_size, @data_directory8_virtual_address, @data_directory8_virtual_size, @data_directory9_virtual_address, @data_directory9_virtual_size, @data_directory10_virtual_address, @data_directory10_virtual_size, @data_directory11_virtual_address, @data_directory11_virtual_size, @data_directory12_virtual_address, @data_directory12_virtual_size, @data_directory13_virtual_address, @data_directory13_virtual_size, @data_directory14_virtual_address, @data_directory14_virtual_size, @data_directory15_virtual_address, @data_directory15_virtual_size = bin.unpack(FORMAT)
    return self
  end

  alias :resource_directory_virtual_address  :data_directory2_virtual_address
  alias :resource_directory_virtual_address= :data_directory2_virtual_address=
  alias :resource_directory_virtual_size  :data_directory2_virtual_size
  alias :resource_directory_virtual_size= :data_directory2_virtual_size=

end # Exerb::Win32::Struct::ImageOptionalHeader32

#==============================================================================#
#==============================================================================#
