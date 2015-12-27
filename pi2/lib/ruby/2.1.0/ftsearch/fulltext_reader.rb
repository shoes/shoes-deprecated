# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

module FTSearch

class FulltextReader
  DEFAULT_OPTIONS = {
    :path => nil,
    :io   => nil,
  }
  def initialize(options = {})
    options = DEFAULT_OPTIONS.merge(options)
    unless options[:path] || options[:io]
      raise ArgumentError, "Need either the path to the suffix array file or an IO."
    end
    init_internal_structures(options)
  end

  def get_data(offset, size)
    @io.pos = offset
    @io.read(size)
  end

  def dump_data(&block)
    blocksize = 32768
    @io.pos = 0
    begin
      size = @io.stat.size - 1
    rescue NoMethodError # try with StringIO's interface
      size = @io.string.size - 1
    end
    read = 0
    #buffer = " " * blocksize
    while read < size
      #@io.read([size - read, blocksize].min, buffer)
      #yield buffer
      data = @io.read([size - read, blocksize].min)
      read += data.size
      yield data
    end
  end

  private
  def init_internal_structures(options)
    if options[:path]
      @io = File.open(options[:path], "rb")
    else
      @io = options[:io]
    end
  end
end
end #  FTSearch
