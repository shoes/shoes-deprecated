
#==============================================================================#
# $Id: pe_file.rb,v 1.5 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

require 'stringio'
require 'exerb/win32/struct/image_dos_header'
require 'exerb/win32/struct/image_nt_headers32'
require 'exerb/win32/struct/image_section_header'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::PeFile

  def initialize
    @dos_header = nil
    @nt_headers = nil
    @sections   = nil
  end

  attr_accessor :dos_header, :nt_headers, :sections

  def self.read(io)
    return self.new.read(io)
  end

  def self.new_from_binary(bin)
    return self.read(StringIO.new(bin))
  end

  def self.new_from_file(filepath)
    return File.open(filepath, 'rb') { |file| self.read(file) }
  end

  def read(io)
    @dos_header = Exerb::Win32::Struct::ImageDosHeader.read(io)
    io.seek(-@dos_header.size, IO::SEEK_CUR)
    io.seek(@dos_header.offset_to_new_header, IO::SEEK_CUR)
    @nt_headers = Exerb::Win32::Struct::ImageNtHeaders32.read(io)
    @sections   = (1..@nt_headers.file_header.number_of_sections).collect { Exerb::Win32::Struct::ImageSectionHeader.read(io) }

    return self
  end

  def find_section(name)
    return @sections.find { |section| section.name == name }
  end

end # Exerb::Win32::PeFile

#==============================================================================#
#==============================================================================#
