
#==============================================================================#
# $Id: resource.rb,v 1.30 2005/05/05 02:45:41 yuya Exp $
#==============================================================================#

require 'stringio'
require 'exerb/error'
require 'exerb/win32/resource_directory'
require 'exerb/win32/resource_directory_root'
require 'exerb/win32/resource_entry'
require 'exerb/win32/pe_file'
require 'exerb/win32/const/resource'
require 'exerb/resource/icon'
require 'exerb/resource/group_icon'
require 'exerb/resource/version_info'
require 'exerb/resource/dialog'
require 'exerb/resource/binary'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::Resource

  include Enumerable

  DEFAULT_LANG_ID = 0x0400 # Neutral
  RT_EXERB = 100
  ID_EXERB = 1

  def initialize
    @entries = {}
  end

  attr_reader :entries

  def self.read(io, base)
    return self.new.read(io, base)
  end

  def self.new_from_binary(bin, base)
    return self.read(StringIO.new(bin), base)
  end

  def self.new_from_pe_binary(bin)
    pe     = Exerb::Win32::PeFile.new_from_binary(bin)
    rsrc   = pe.sections.find { |sec| sec.name == '.rsrc' }
    raise(ExerbError, "a resource section was not found in the core") if rsrc.nil?

    return self.new_from_binary(bin[rsrc.pointer_to_raw_data, rsrc.virtual_size], rsrc.virtual_address)
  end

  def self.new_from_pe_file(filepath)
    return File.open(filepath) { |file| self.new_from_pe_binary(file.read) }
  end

  def each
    @entries.keys.sort.each { |type|
      @entries[type].keys.sort.each { |id|
        @entries[type][id].keys.sort.each { |lang|
          yield(@entries[type][id][lang])
        }
      }
    }
  end

  def add(type, id, data, lang = DEFAULT_LANG_ID)
    @entries[type] ||= {}
    @entries[type][id] ||= {}
    @entries[type][id][lang] = Exerb::Resource::Entry.new(type, id, lang, data)

    return self
  end

  def add_icon(id, data, lang = DEFAULT_LANG_ID)
    return self.add(Exerb::Win32::Const::RT_ICON, id, data, lang)
  end

  def add_group_icon(id, data, lang = DEFAULT_LANG_ID)
    return self.add(Exerb::Win32::Const::RT_GROUP_ICON, id, data, lang)
  end

  def add_dialog(id, data, lang = DEFAULT_LANG_ID)
    return self.add(Exerb::Win32::Const::RT_DIALOG, id, data, lang)
  end

  def add_version(id, data, lang = DEFAULT_LANG_ID)
    return self.add(Exerb::Win32::Const::RT_VERSION, id, data, lang)
  end

  def add_archive(archive)
    return self.add(Exerb::Resource::RT_EXERB, Exerb::Resource::ID_EXERB, archive)
  end

  def pack(base, reloc = [])
    root_dir = Exerb::Win32::ResourceDirectoryRoot.new

    @entries.keys.sort.each { |type|
      root_dir << Exerb::Win32::ResourceDirectory.new(type) { |type_dir|
        @entries[type].keys.sort.each { |id|
          type_dir << Exerb::Win32::ResourceDirectory.new(id) { |item_dir|
            @entries[type][id].keys.sort.each { |lang|
              item_dir << @entries[type][id][lang].to_resource_entry
            }
          }
        }
      }
    }

    return root_dir.pack_all(base, reloc)
  end

  def read(io, base)
    root_dir = Exerb::Win32::ResourceDirectoryRoot.read(io, base)
    root_dir.entries.each { |type_dir|
      type_dir.entries.each { |item_dir|
        item_dir.entries.each { |item|
          type, id, lang = type_dir.name, item_dir.name, item.lang
          @entries[type] ||= {}
          @entries[type][id] ||= {}
          @entries[type][id][lang] = Exerb::Resource::Entry.new(type, id, lang, Exerb::Resource::Binary.new(item.entry_data.data))
        }
      }
    }

    return self
  end

  def remove(type, id = nil, lang = nil)
    if type && id && lang
      if @entries[type] && @entries[type][id] && @entries[type][id][lang]
        @entries[type][id].delete(lang)
      end
    elsif type && id
      if @entries[type] && @entries[type][id]
        @entries[type].delete(id)
      end
    elsif type
      if @entries[type]
        @entries.delete(type)
      end
    end

    if type && id && @entries[type] && @entries[type][id] && @entries[type][id].empty?
      @entries[type].delete(id)
    end

    if type && @entries[type] && @entries[type].empty?
      @entries.delete(type)
    end

    return self
  end

  def merge(res)
    res.each { |entry|
      @entries[entry.type] ||= {}
      @entries[entry.type][entry.id] ||= {}
      @entries[entry.type][entry.id][entry.lang] = entry
    }

    return self
  end

end # Exerb::Resource

#==============================================================================#

class Exerb::Resource::Entry

  def initialize(type, id, lang, data)
    @type = type
    @id   = id
    @lang = lang
    @data = data
  end

  attr_accessor :type, :id, :lang, :data

  def to_resource_entry
    return Exerb::Win32::ResourceEntry.new(@data.pack, lang)
  end

end # Exerb::Resource::Entry

#==============================================================================#
#==============================================================================#
