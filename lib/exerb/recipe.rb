#! /usr/bin/ruby

#==============================================================================#
# $Id: recipe.rb,v 1.24 2007/02/26 10:59:31 yuya Exp $
#==============================================================================#

require 'yaml'
require 'exerb/error'
require 'exerb/utility'
require 'exerb/archive'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::Recipe

  def initialize(blocks, basedir = Dir.pwd, filename = '-')
    @basedir  = basedir
    @filename = filename
    @general  = Exerb::Recipe::GeneralBlock.analyze(blocks, @filename)
    @path     = Exerb::Recipe::PathBlock.analyze(blocks, @filename)
    @resource = Exerb::Recipe::ResourceBlock.analyze(blocks, @filename)
    @file     = Exerb::Recipe::FileBlock.analyze(blocks, @filename)
    raise(Exerb::ExerbError, "#{@filename}: found unknown field in the recipe -- #{blocks.keys.join(', ')}") unless blocks.empty?
  end

  def self.load(filepath)
    basedir  = File.dirname(File.expand_path(filepath))
    filename = File.basename(filepath)

    begin
      blocks = YAML.load(File.read(filepath))
    rescue Errno::ENOENT => e
      raise(Exerb::ExerbError, "no such file -- #{filepath}")
    rescue ArgumentError => e
      msg = e.message.gsub(/\n/, '\n')
      msg.gsub!(/\r/, '\r')
      msg.gsub!(/\t/, '\t')
      raise(Exerb::ExerbError, format('%s: %s', @filename, msg))
    end

    return self.new(blocks, basedir, filename)
  end

  def archive_filepath(filename = nil)
    return self.output_filepath(filename).sub(/(\.exe)?$/i, '.exa')
  end

  def output_filepath(filename = nil)
    filename ||= @general.output
    filename ||= @filename.sub(/(\.exy)?$/i, '.exe')

    return File.expand_path(filename, @basedir)
  end

  def core_filepath(filepath = nil)
    unless @general.core.nil?
      core   = Exerb::Utility.find_core_by_filepath(File.expand_path(@general.core, @basedir))
      core ||= Exerb::Utility.find_core_by_filename(@general.core)
      core ||= Exerb::Utility.find_core_by_name(@general.core)
      raise(Exerb::ExerbError, "#{@filename}: specified core isn't found -- #{@general.core}") if core.nil?
    end

    filepath ||= core
    filepath ||= Exerb::Utility.find_core_by_name('cui', true)

    return filepath
  end

  def create_archive(kcode = nil)
    archive = Exerb::Archive.new(kcode || @general.kcode)

    search_path  = [@basedir]
    search_path += @path.path.collect { |dir| File.expand_path(dir, @basedir) } if @path
    search_path += $:

    startup_entry = @file.delete(@general.startup)
    raise(Exerb::ExerbError, "#{@filename}: specified startup script isn't found in the file block -- #{@general.startup}") if startup_entry.nil?
    add_file_entry(archive, search_path, @general.startup, startup_entry)

    @file.keys.sort.each { |internal_name|
      add_file_entry(archive, search_path, internal_name, @file[internal_name])
    }

    return archive
  end

  def update_resource(rsrc)
    if @resource
      if @resource.icon
        rsrc.remove(Exerb::Win32::Const::RT_ICON)
        rsrc.remove(Exerb::Win32::Const::RT_GROUP_ICON)

        group_icon = Exerb::Resource::GroupIcon.new

        @resource.icon.entries.each_with_index { |entry, index|
          id   = index + 1
          icon = Exerb::Resource::Icon.read(entry.file, entry.width, entry.height, entry.color)
          rsrc.add(Exerb::Win32::Const::RT_ICON, id, icon)
          group_icon.add(id, icon)
        }

        rsrc.add(Exerb::Win32::Const::RT_GROUP_ICON, 1, group_icon)
      end

      if @resource.version
        fv = (0..3).collect { |i| @resource.version.file_version_number[i]    || 0 }
        pv = (0..3).collect { |i| @resource.version.product_version_number[i] || 0 }

        ver = Exerb::Resource::VersionInfo.new
        ver.file_version_number    = Exerb::Resource::VersionInfo.make_version(*fv)
        ver.product_version_number = Exerb::Resource::VersionInfo.make_version(*pv)
        ver.comments               = @resource.version.comments
        ver.company_name           = @resource.version.company_name
        ver.legal_copyright        = @resource.version.legal_copyright
        ver.legal_trademarks       = @resource.version.legal_trademarks
        ver.file_version           = @resource.version.file_version
        ver.product_version        = @resource.version.product_version
        ver.product_name           = @resource.version.product_name
        ver.file_description       = @resource.version.file_description
        ver.internal_name          = @resource.version.internal_name
        ver.original_filename      = @resource.version.original_filename
        ver.private_build          = @resource.version.private_build
        ver.special_build          = @resource.version.special_build

        rsrc.remove(Exerb::Win32::Const::RT_VERSION)
        rsrc.add(Exerb::Win32::Const::RT_VERSION, 1, ver)
      end
    end
  end

  def add_file_entry(archive, search_path, internal_name, entry)
    if entry.file
      filepath = File.expand_path(entry.file, @basedir)
      raise("#{@filename}: no such file -- #{entry.file}") unless File.exist?(filepath)
    else
      filepath = search_path.collect { |dir|
        File.join(dir, internal_name)
      }.find { |path|
        File.exist?(path)
      }
      raise("#{@filename}: no such file -- #{internal_name}") unless filepath
    end

    flag = ([entry.type] + entry.flag).inject(0) { |a, b| a | b }

    if entry.type == Exerb::FileTable::Entry::FLAG_TYPE_COMPILED_SCRIPT
      src  = File.open(filepath, "rb") { |file| file.read }
      iseq = VM::InstructionSequence.compile(src)
      bin  = Marshal.dump(iseq.to_a)
      archive.add(internal_name, bin, flag)
    else
      archive.add_from_file(internal_name, filepath, flag)
    end
  end
  protected :add_file_entry

end # Exerb::Recipe

#==============================================================================#

class Exerb::Recipe::GeneralBlock

  KCODE = ['none', 'euc', 'sjis', 'utf8']

  def initialize(block, filename)
    @startup = block.delete('startup') || ""
    @output  = block.delete('output')  || ""
    @core    = block.delete('core')    || ""
    @kcode   = block.delete('kcode')   || "none"

    @startup = nil if @startup.empty?
    @output  = nil if @output.empty?
    @core    = nil if @core.empty?
    @kcode.downcase!

    raise(Exerb::ExerbError, "#{filename}: unknown field at general block -- #{block.keys.join(', ')}") unless block.empty?
    raise(Exerb::ExerbError, "#{filename}: startup field isn't specified at general block") if @startup.nil?
    raise(Exerb::ExerbError, "#{filename}: unknown kcode at general block -- #{@kcode}") unless KCODE.include?(@kcode)
  end

  attr_reader :startup, :output, :core, :kcode

  def self.analyze(blocks, filename)
    block = blocks.delete("general")
    raise(Exerb::ExerbError, "#{filename}: general block not found") if block.nil?
    raise(Exerb::ExerbError, "#{filename}: general block must be Hash object -- #{block.class}") unless block.kind_of?(Hash)
    return self.new(block, filename)
  end

end # Exerb::Recipe::GeneralBlock

#==============================================================================#

class Exerb::Recipe::PathBlock

  def initialize(block, filename)
    @path = block.collect { |entry|
      raise(Exerb::ExerbError, "#{filename}: path block entries must be String object -- #{entry.type}") unless entry.kind_of?(String)
      entry.strip
    }
  end

  attr_reader :path

  def self.analyze(blocks, filename)
    block = blocks.delete("path")
    return nil if block.nil?
    raise(Exerb::ExerbError, "#{filename}: path block must be Array object -- #{block.class}") unless block.kind_of?(Array)
    return self.new(block, filename)
  end

end # Exerb::Recipe::PathBlock

#==============================================================================#

class Exerb::Recipe::ResourceBlock

  def initialize(block, filename)
    @icon    = IconBlock.analyze(block, filename)
    @version = VersionBlock.analyze(block, filename)
    raise(Exerb::ExerbError, "#{filename}: unknown field at resource block -- #{block.keys.join(', ')}") unless block.empty?
  end

  attr_reader :icon, :version

  def self.analyze(blocks, filename)
    block = blocks.delete("resource")
    return nil if block.nil?
    raise(Exerb::ExerbError, "#{filename}: resource block must be Hash object -- #{block.type}") unless block.kind_of?(Hash)
    return self.new(block, filename)
  end

  class IconBlock

    def initialize(block, filename)
      @entries = block.collect { |hash| Entry.analyze(hash, filename) }
    end

    attr_reader :entries

    def self.analyze(blocks, filename)
      block = blocks.delete("icon")
      return nil if block.nil?
      raise(Exerb::ExerbError, "#{filename}: icon block must be Array object -- #{block.type}") unless block.kind_of?(Array)
      return self.new(block, filename)
    end

    class Entry

      def initialize(block, filename)
        @width  = block.delete("width")
        @height = block.delete("height")
        @color  = block.delete("color")
        @file   = block.delete("file")

        raise(Exerb::ExerbError, "#{filename}: unknown field at icon block entry -- #{block.keys.join(', ')}") unless block.empty?
      end

      attr_reader :width, :height, :color, :file

      def self.analyze(block, filename)
        raise(Exerb::ExerbError, "#{filename}: icon block entry must be Hash object -- #{block.type}") unless block.kind_of?(Hash)
        return self.new(block, filename)
      end

    end # Entry
  end # IconBlock

  class VersionBlock

    def initialize(block, filename)
      @file_version_number    = self.parse_version((block.delete('file_version_number')    || "0.0.0.0"), filename)
      @product_version_number = self.parse_version((block.delete('product_version_number') || "0.0.0.0"), filename)
      @comments               = block.delete('comments')          || ""
      @company_name           = block.delete('company_name')      || ""
      @legal_copyright        = block.delete('legal_copyright')   || ""
      @legal_trademarks       = block.delete('legal_trademarks')  || ""
      @file_version           = block.delete('file_version')      || ""
      @product_version        = block.delete('product_version')   || ""
      @product_name           = block.delete('product_name')      || ""
      @file_description       = block.delete('file_description')  || ""
      @internal_name          = block.delete('internal_name')     || ""
      @original_filename      = block.delete('original_filename') || ""
      @private_build          = block.delete('private_build')     || ""
      @special_build          = block.delete('special_build')     || ""

      raise(Exerb::ExerbError, "#{filename}: unknown field in version block -- #{block.keys.join(', ')}") unless block.empty?
    end

    attr_reader :file_version_number, :product_version_number,
      :comments, :company_name, :legal_copyright, :legal_trademarks, :file_version,
      :product_version, :product_name, :file_description, :internal_name, :original_filename,
      :private_build, :special_build

    def self.analyze(blocks, filename)
      block = blocks.delete("version")
      return nil if block.nil?
      raise(Exerb::ExerbError, "#{filename}: version block must be Hash object -- #{block.class}") unless block.kind_of?(Hash)
      return self.new(block, filename)
    end

    def parse_version(version, filename)
      case version.delete(" ").tr(",", ".")
      when /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/ then return [$1.to_i, $2.to_i, $3.to_i, $4.to_i]
      when /^(\d+)\.(\d+)\.(\d+)$/        then return [$1.to_i, $2.to_i, $3.to_i, 0]
      when /^(\d+)\.(\d+)$/               then return [$1.to_i, $2.to_i, 0,       0]
      when /^(\d+)$/                      then return [$1.to_i, 0,       0,       0]
      when ""                             then return [0,       0,       0,       0]
      else raise(Exerb::ExerbError, "#{filename}: invalid version format in version block -- #{version}")
      end
    end

  end # VersionBlock

end # Exerb::Recipe::ResourceBlock

#==============================================================================#

class Exerb::Recipe::FileBlock

  include Enumerable

  def initialize(block, filename)
    @file = {}

    block.each { |internal_name, options|
      @file[internal_name] = FileEntry.new(internal_name, options, filename)
    }
  end

  def self.analyze(blocks, filename)
    block = blocks.delete("file")
    raise(Exerb::ExerbError, "#{filename}: file block not found") if block.nil?
    raise(Exerb::ExerbError, "#{filename}: file block must be Hash object -- #{block.class}") unless block.kind_of?(Hash)
    return self.new(block, filename)
  end

  def [](key)
    return @file[key]
  end

  def each(&block)
    return @file.each(&block)
  end

  def keys
    return @file.keys
  end

  def delete(key)
    return @file.delete(key)
  end

end # Exerb::Recipe::FileBlock

#==============================================================================#

class Exerb::Recipe::FileBlock::FileEntry

  def initialize(internal_name, options, filename)
    @file = nil
    @type = Exerb::FileTable::Entry::FLAG_TYPE_RUBY_SCRIPT
    @flag = []
    self.analyze_options(internal_name, options || {}, filename)
  end

  attr_reader :file, :type, :flag

  def analyze_options(internal_name, options, filename)
    raise(Exerb::ExerbError, "#{filename}: file entry must be Hash object -- #{options.class}") unless options.kind_of?(Hash)
    @file = self.analyze_option_file(options, filename)
    @type = self.analyze_option_type(internal_name, options, filename)
    @flag = self.analyze_option_flag(options, filename, @type)
  end
  protected :analyze_options

  def analyze_option_file(options, filename)
    return options['file']
  end
  protected :analyze_option_file

  def analyze_option_type(internal_name, options, filename)
    default_type = 
      case internal_name
      when /\.rb/i  then Exerb::FileTable::Entry::FLAG_TYPE_RUBY_SCRIPT
      when /\.so/i  then Exerb::FileTable::Entry::FLAG_TYPE_EXTENSION_LIBRARY
      when /\.dll/i then Exerb::FileTable::Entry::FLAG_TYPE_DYNAMIC_LIBRARY
      when /\.res/i then Exerb::FileTable::Entry::FLAG_TYPE_RESOURCE_LIBRARY
      when /\.dat/i then Exerb::FileTable::Entry::FLAG_TYPE_DATA_BINARY
      else               Exerb::FileTable::Entry::FLAG_TYPE_RUBY_SCRIPT
      end

    type = options['type'].to_s.downcase

    case type
    when ''                  then return default_type
    when 'script'            then return Exerb::FileTable::Entry::FLAG_TYPE_RUBY_SCRIPT
    when 'extension-library' then return Exerb::FileTable::Entry::FLAG_TYPE_EXTENSION_LIBRARY
    when 'dynamic-library'   then return Exerb::FileTable::Entry::FLAG_TYPE_DYNAMIC_LIBRARY
    when 'resource-library'  then return Exerb::FileTable::Entry::FLAG_TYPE_RESOURCE_LIBRARY
    when 'data'              then return Exerb::FileTable::Entry::FLAG_TYPE_DATA_BINARY
    when 'compiled-script'   then return Exerb::FileTable::Entry::FLAG_TYPE_COMPILED_SCRIPT
    else raise(Exerb::ExerbError, "#{filename}: [#{type}] is unknown type at file entry")
    end
  end
  protected :analyze_option_type

  def analyze_option_flag(options, filename, type)
    case options['flag']
    when nil    then flag = []
    when String then flag = [options['flag']]
    when Array  then flag = options['flag']
    else raise(Exerb::ExerbError, "#{filename}: flag field of file entry must be String or Array object")
    end

    known_flags = 
      case type
      when Exerb::FileTable::Entry::FLAG_TYPE_RUBY_SCRIPT       then {} # {'compile_by_bruby' => ...}
      when Exerb::FileTable::Entry::FLAG_TYPE_EXTENSION_LIBRARY then {'no_replace_function' => Exerb::FileTable::Entry::FLAG_NO_REPLACE_FUNCTION}
      when Exerb::FileTable::Entry::FLAG_TYPE_DYNAMIC_LIBRARY   then {}
      when Exerb::FileTable::Entry::FLAG_TYPE_RESOURCE_LIBRARY  then {}
      when Exerb::FileTable::Entry::FLAG_TYPE_DATA_BINARY       then {}
      when Exerb::FileTable::Entry::FLAG_TYPE_COMPILED_SCRIPT   then {}
      end

    selected_flags = known_flags.keys.select { |name|
      flag.delete(name)
    }.collect { |name|
      known_flags[name]
    }

    unless flag.empty?
      raise(Exerb::ExerbError, "#{filename}: found unknown flag in file entry -- #{flag.join(', ')}")
    end

    return selected_flags
  end
  protected :analyze_option_flag

end # Exerb::Recipe::FileBlock::FileEntry

#==============================================================================#

if $0 == __FILE__
  recipe = Exerb::Recipe.load(ARGV.shift)
  p(recipe)
  p(recipe.archive_filepath)
  p(recipe.output_filepath)
  p(recipe.core_filepath)
  recipe.create_archive.write_to_file('tmp.exa')
end

#==============================================================================#
#==============================================================================#
