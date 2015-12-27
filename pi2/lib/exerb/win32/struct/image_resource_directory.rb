
#==============================================================================#
# $Id: image_resource_directory.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
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

class Exerb::Win32::Struct::ImageResourceDirectory < Exerb::Win32::Struct::Base

  FORMAT = 'LLSSSS'

  def initialize
    super()
    @characteristics        = 0
    @time_date_stamp        = 0
    @major_version          = 0
    @minor_version          = 0
    @number_of_name_entries = 0
    @number_of_id_entries   = 0
  end

  attr_accessor :characteristics, :time_date_stamp, :major_version, :minor_version, :number_of_name_entries, :number_of_id_entries

  def pack
    return [@characteristics, @time_date_stamp, @major_version, @minor_version, @number_of_name_entries, @number_of_id_entries].pack(FORMAT)
  end

  def unpack(bin)
    @characteristics, @time_date_stamp, @major_version, @minor_version, @number_of_name_entries, @number_of_id_entries = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::ImageResourceDirectory

#==============================================================================#
#==============================================================================#
