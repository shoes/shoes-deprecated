# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'ftsearch/util'

module FTSearch
class DocumentMapWriter
  include InMemoryWriter

  DEFAULT_OPTIONS = {
    :path => "docmap-#{Process.pid}-#{rand(100000)}",
  }
  def initialize(options = {})
    options    = DEFAULT_OPTIONS.merge(options)
    @path      = options[:path]
    @field_arr = []
    @uri_tbl   = []
    @data      = [@uri_tbl, @field_arr]
    initialize_in_memory_buffer
  end

  def merge(doc_map_reader)
    # TODO: general merge?
    @uri_tbl, @field_arr = doc_map_reader.dump_data
    @data = [@uri_tbl, @field_arr]
  end

  def add_document(doc_id, uri)
    @uri_tbl[doc_id] = uri
  end

  def add_field(offset, doc_id, field_id, size)
    @field_arr << [offset, doc_id, field_id, size]
  end

  def finish!
    if @path
      File.open(@path, "wb") do |f|
        Marshal.dump(@data, f)
      end
    else
      Marshal.dump(@data, @memory_io)
    end
  end
end
end  # FTSearch
