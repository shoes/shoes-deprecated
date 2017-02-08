# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'fileutils'
require 'ftsearch/suffix_array_writer'
require 'ftsearch/document_map_writer'
require 'ftsearch/fulltext_writer'
require 'ftsearch/field_infos'

require 'ftsearch/fulltext_reader'
require 'ftsearch/document_map_reader'
require 'ftsearch/suffix_array_reader'

module FTSearch

class FragmentWriter
  DEFAULT_OPTIONS = {
    :path                      => "ftsearch-#{Process.pid}-#{rand(100000)}",
    :default_analyzer_class    => FTSearch::Analysis::WhiteSpaceAnalyzer,
    :field_infos_class         => FieldInfos,
    :fulltext_writer_class     => FulltextWriter,
    :suffix_array_writer_class => SuffixArrayWriter,
    :doc_map_writer_class      => DocumentMapWriter,
    :field_infos               => nil,
    :fulltext_writer           => nil,
    :suffix_array_writer       => nil,
    :doc_map_writer_nil        => nil,
  }

  attr_reader :fulltext_writer, :suffix_array_writer, :doc_map_writer
  def initialize(options = {})
    options = DEFAULT_OPTIONS.merge(options)
    create  = lambda do |name, *args|
      options[name] || options[(name.to_s + "_class").to_sym].new(*args)
    end
    build_path = lambda do |suffix|
      if @path
        File.join(@tmpdir, suffix)
      else
        nil
      end
    end
    @path = options[:path]
    if @path
      @path   = File.expand_path(@path)
      @tmpdir = @path + "#{Process.pid}-#{rand(100000)}"
      FileUtils.mkdir_p(@tmpdir)
    end

    @fulltext_writer     = create.call(:fulltext_writer, :path     => build_path["fulltext"])
    @suffix_array_writer = create.call(:suffix_array_writer, :path => build_path["suffixes"])
    @doc_map_writer      = create.call(:doc_map_writer, :path      => build_path["docmap"])

    default_analyzer = (klass = options[:default_analyzer_class]) ? klass.new : nil
    @field_infos     = create.call(:field_infos, :default_analyzer => default_analyzer)
    @num_documents   = 0
    @field_map       = Hash.new{|h,k| h[k.to_sym] = h.size}
    @field_map[:uri] # init
  end

  def add_document(doc_hash)
    uri = doc_hash[:uri] || @num_documents.to_s
    @fulltext_writer.add_document(@num_documents, doc_hash.merge(:uri => uri), 
                                  @field_map, @field_infos, @suffix_array_writer, @doc_map_writer)
    @num_documents += 1
  end

  def merge(fragment_directory)
    raise "Cannot import old data unless the destination Fragment is empty." unless @num_documents == 0
    # TODO: use a FragmentReader to access old data
    fulltext_reader     = FulltextReader.new(:path => "#{fragment_directory}/fulltext")
    suffix_array_reader = SuffixArrayReader.new(fulltext_reader, nil, 
                                                :path => "#{fragment_directory}/suffixes")
    doc_map_reader      = DocumentMapReader.new(:path => "#{fragment_directory}/docmap")
    @fulltext_writer.merge(fulltext_reader)
    @suffix_array_writer.merge(suffix_array_reader)
    @doc_map_writer.merge(doc_map_reader)
    #FIXME: .num_documents will be wrong if some URIs were repeated
    @num_documents = doc_map_reader.num_documents
    File.open(File.join(fragment_directory, "fieldmap"), "rb") do |f|
      i = 0
      f.each_line{|l| @field_map[l.chomp.to_sym] = i; i+= 1}
    end
  end

  def fields
    @field_map.sort_by{|field, fid| fid}.map{|field, fid| field}
  end

  def documents
    @num_documents
  end

  def field_id(field)
    @field_map.has_key?(field) && @field_map[field]
  end

  def finish!
    @fulltext_writer.finish!
    fulltext = @fulltext_writer.data
    if $DEBUG
      $DEBUGF.puts "FragFinish! #{fulltext.size} #{fulltext.bytesize}"
      $DEBUGF.flush
    end
    @suffix_array_writer.finish!(fulltext)
    @doc_map_writer.finish!

    if @path
      File.open(File.join(@tmpdir, "fieldmap"), "wb") do |f|
        @field_map.sort_by{|field_name, field_id| field_id}.each do |field_name, field_id| 
          f.puts field_name
        end
        File.rename(@tmpdir, @path)
      end
    end
  end
end
end  # FTSearch
