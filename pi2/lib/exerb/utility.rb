
#==============================================================================#
# $Id: utility.rb,v 1.15 2005/05/03 13:47:01 yuya Exp $
#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

module Exerb::Utility

  def self.align_value(value, align)
    if value % align == 0
      return value
    else
      return value + (align - (value % align))
    end
  end

  def self.alignment(bin, align)
    if bin.size % align == 0
      return bin
    else
      return bin + "\0" * (align - (bin.size % align))
    end
  end

  def self.alignment16(bin)
    return alignment(bin, 16)
  end

  def self.alignment4k(bin)
    return alignment(bin, 1024 * 4)
  end

  def self.find_file_by_filename(filename, path)
    return filename if File.expand_path(filename) == filename && File.exist?(filename)
    return path.collect { |dir| File.join(dir, filename) }.find { |filepath| File.exist?(filepath) }
  end

  def self.find_file_by_name(name, table, path)
    return nil unless table.has_key?(name)
    return self.find_file_by_filename(table[name], path)
  end

  def self.find_core_by_filepath(filepath)
    return (File.exist?(filepath) ? filepath : nil)
  end

  def self.find_core_by_filename(filename)
    require 'exerb/config'
    return self.find_file_by_filename(filename, Exerb::CORE_PATH)
  end

  def self.find_core_by_name(name, error = false)
    require 'exerb/config'
    path = self.find_file_by_name(name, Exerb::CORE_NAME, Exerb::CORE_PATH)
    raise(Exerb::ExerbError, "unknown core name -- #{name}") if path.nil? && error
    return path
  end

  def self.read(input)
    case input
    when IO
      input.binmode
      return input.read
    when String
      return File.open(input, "rb") { |file| self.read(file) }
    else raise(ArgumentError, "input must be IO or String object")
    end
  end

  def self.write(output, binary, mode = nil)
    case output
    when IO
      output.binmode
      output.write(binary)
    when String
      File.open(output, "wb") { |file| self.write(file, binary) }
      File.chmod(mode, output) if mode
    else raise(ArgumentError, "output must be IO or String object")
    end
    return nil
  end

end # Exerb::Utility

#==============================================================================#

if $0 == __FILE__
end

#==============================================================================#
#==============================================================================#
