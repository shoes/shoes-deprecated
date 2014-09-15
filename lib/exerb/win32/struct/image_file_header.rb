
#==============================================================================#
# $Id: image_file_header.rb,v 1.3 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::ImageFileHeader < Exerb::Win32::Struct::Base

  FORMAT = 'SSLLLSS'

  def initialize
    @machine                 = 0
    @number_of_sections      = 0
    @time_date_stamp         = 0
    @pointer_to_symbol_table = 0
    @number_of_symbols       = 0
    @size_of_optional_header = 0
    @characteristics         = 0
  end

  attr_accessor :machine, :number_of_sections, :time_date_stamp, :pointer_to_symbol_table, :number_of_symbols, :size_of_optional_header, :characteristics

  def pack
    return [@machine, @number_of_sections, @time_date_stamp, @pointer_to_symbol_table, @number_of_symbols, @size_of_optional_header, @characteristics].pack(FORMAT)
  end

  def unpack(bin)
    @machine, @number_of_sections, @time_date_stamp, @pointer_to_symbol_table, @number_of_symbols, @size_of_optional_header, @characteristics = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::ImageFileHeader

#==============================================================================#
#==============================================================================#
