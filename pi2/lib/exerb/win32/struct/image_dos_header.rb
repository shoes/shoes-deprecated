
#==============================================================================#
# $Id: image_dos_header.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::ImageDosHeader < Exerb::Win32::Struct::Base

  FORMAT    = 'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSL'
  SIGNATURE = 0x5A4D

  def initialize
    @magic                       = 0
    @last_page_size              = 0
    @total_pages_in_file         = 0
    @relocation_items            = 0
    @paragraphs_in_header        = 0
    @minimum_extra_paragraphs    = 0
    @maximum_extra_paragraphs    = 0
    @initial_stack_segment       = 0
    @initial_stack_pointer       = 0
    @complemented_checksum       = 0
    @initial_instraction_pointer = 0
    @initial_code_segment        = 0
    @relocation_table_offset     = 0
    @overlay_number              = 0
    @reserved1_1                 = 0
    @reserved1_2                 = 0
    @reserved1_3                 = 0
    @reserved1_4                 = 0
    @oem_identifier              = 0
    @oem_information             = 0
    @reserved2_1                 = 0
    @reserved2_2                 = 0
    @reserved2_3                 = 0
    @reserved2_4                 = 0
    @reserved2_5                 = 0
    @reserved2_6                 = 0
    @reserved2_7                 = 0
    @reserved2_8                 = 0
    @reserved2_9                 = 0
    @reserved2_10                = 0
    @offset_to_new_header        = 0
  end

  attr_accessor :magic, :last_page_size, :total_pages_in_file, :relocation_items, :paragraphs_in_header, :minimum_extra_paragraphs, :maximum_extra_paragraphs, :initial_stack_segment, :initial_stack_pointer, :complemented_checksum, :initial_instraction_pointer, :initial_code_segment, :relocation_table_offset, :overlay_number, :reserved1_1, :reserved1_2, :reserved1_3, :reserved1_4, :oem_identifier, :oem_information, :reserved2_1, :reserved2_2, :reserved2_3, :reserved2_4, :reserved2_5, :reserved2_6, :reserved2_7, :reserved2_8, :reserved2_9, :reserved2_10, :offset_to_new_header

  def pack
    return [@magic, @last_page_size, @total_pages_in_file, @relocation_items, @paragraphs_in_header, @minimum_extra_paragraphs, @maximum_extra_paragraphs, @initial_stack_segment, @initial_stack_pointer, @complemented_checksum, @initial_instraction_pointer, @initial_code_segment, @relocation_table_offset, @overlay_number, @reserved1_1, @reserved1_2, @reserved1_3, @reserved1_4, @oem_identifier, @oem_information, @reserved2_1, @reserved2_2, @reserved2_3, @reserved2_4, @reserved2_5, @reserved2_6, @reserved2_7, @reserved2_8, @reserved2_9, @reserved2_10, @offset_to_new_header].pack(FORMAT)
  end

  def unpack(bin)
    @magic, @last_page_size, @total_pages_in_file, @relocation_items, @paragraphs_in_header, @minimum_extra_paragraphs, @maximum_extra_paragraphs, @initial_stack_segment, @initial_stack_pointer, @complemented_checksum, @initial_instraction_pointer, @initial_code_segment, @relocation_table_offset, @overlay_number, @reserved1_1, @reserved1_2, @reserved1_3, @reserved1_4, @oem_identifier, @oem_information, @reserved2_1, @reserved2_2, @reserved2_3, @reserved2_4, @reserved2_5, @reserved2_6, @reserved2_7, @reserved2_8, @reserved2_9, @reserved2_10, @offset_to_new_header = bin.unpack(FORMAT)
    raise("dos header has invalid signature") unless @magic == SIGNATURE
    return self
  end

end # Exerb::Win32::Struct::ImageDosHeader

#==============================================================================#
#==============================================================================#
