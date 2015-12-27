
#==============================================================================#
# $Id: archive.rb,v 1.38 2007/02/26 10:59:31 yuya Exp $
#==============================================================================#

require 'exerb/name_table'
require 'exerb/file_table'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::Archive

  def initialize(kcode = 'none')
    @name_table   = Exerb::NameTable.new
    @file_table   = Exerb::FileTable.new
    @kcode        = kcode
  end

  attr_reader   :name_table, :file_table
  attr_accessor :kcode

  def add(name, data, flag = 0)
    @file_table.add(@name_table.add(name), data, flag)
  end

  def add_from_file(name, path, flag = 0)
    @file_table.add_from_file(@name_table.add(name), path, flag)
  end

  def add_ruby_script(name, data, flag = 0)
    @file_table.add_ruby_script(@name_table.add(name), data, flag)
  end

  def add_ruby_script_from_file(name, path, flag = 0)
    @file_table.add_ruby_script_from_file(@name_table.add(name), path, flag)
  end

  def add_extension_library(name, data, flag = 0)
    @file_table.add_extension_library(@name_table.add(name), data, flag)
  end

  def add_extension_library_from_file(name, path, flag = 0)
    @file_table.add_extension_library_from_file(@name_table.add(name), path, flag)
  end

  def add_dynamic_library(name, data, flag = 0)
    @file_table.add_dynamic_library(@name_table.add(name), data, flag)
  end

  def add_dynamic_library_from_file(path, flag = 0)
    @file_table.add_dynamic_library_from_file(@name_table.add(File.basename(path)), path, flag)
  end

  def add_resource_library(name, data, flag = 0)
    @file_table.add_resource_library(@name_table.add(name), data, flag)
  end

  def add_resource_library_from_file(path, flag = 0)
    @file_table.add_resource_library_from_file(@name_table.add(File.basename(path)), path, flag)
  end

  def add_data_binary(name, data, flag = 0)
    @file_table.add_data_binary(@name_table.add(name), data, flag)
  end

  def add_data_binary_from_file(name, path, flag = 0)
    @file_table.add_data_binary_from_file(@name_table.add(name), path, flag)
  end

  def add_compiled_script(name, data, flag = 0)
    @file_table.add_compiled_script(@name_table.add(name), data, flag)
  end

  def add_compiled_script_from_file(name, path, flag = 0)
    @file_table.add_compiled_script_from_file(@name_table.add(name), path, flag)
  end

  def pack
    case @kcode.downcase.delete('-')
    when 'n', 'none' then options = Exerb::Archive::Header::OPTIONS_KCODE_NONE
    when 'e', 'euc'  then options = Exerb::Archive::Header::OPTIONS_KCODE_EUC
    when 's', 'sjis' then options = Exerb::Archive::Header::OPTIONS_KCODE_SJIS
    when 'u', 'utf8' then options = Exerb::Archive::Header::OPTIONS_KCODE_UTF8
    else raise(ExerbError, "unkown kanji code [#{@kcode}]")
    end

    packed_name_table   = @name_table.pack
    packed_file_table   = @file_table.pack

    archive_header = Exerb::Archive::Header.new
    archive_header.signature1             = Exerb::Archive::Header::SIGNATURE1
    archive_header.signature2             = Exerb::Archive::Header::SIGNATURE2
    archive_header.options                = options
    archive_header.offset_of_name_table   = Exerb::Utility.alignment16(archive_header.pack).size
    archive_header.offset_of_file_table   = archive_header.offset_of_name_table + packed_name_table.size

    return Exerb::Utility.alignment16(archive_header.pack) + packed_name_table + packed_file_table
  end

  def write_to(io)
    io.write(self.pack)
  end

  def write_to_file(filepath)
    File.open(filepath, 'wb') { |file| self.write_to(file) }
  end

end # Exerb::Archive

#==============================================================================#

class Exerb::Archive::Header

  SIGNATURE1         = 0x52455845
  SIGNATURE2         = 0x04000042
  OPTIONS_KCODE_NONE = 0
  OPTIONS_KCODE_EUC  = 1
  OPTIONS_KCODE_SJIS = 2
  OPTIONS_KCODE_UTF8 = 3

  def initialize
    @signature1           = 0
    @signature2           = 0
    @options              = 0
    @offset_of_name_table = 0
    @offset_of_file_table = 0
  end

  attr_accessor :signature1, :signature2, :options, :offset_of_name_table, :offset_of_file_table

  def pack
    return [@signature1, @signature2, @options, @offset_of_name_table, @offset_of_file_table].pack('LLLLL')
  end

end # Exerb::Archive::Header

#==============================================================================#
#==============================================================================#
