# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'enumerator'
require 'ftsearch/util'

module FTSearch

class SuffixArrayWriter
  include InMemoryWriter

  DEFAULT_OPTIONS = {
    :path => "suffixes-#{Process.pid}-#{rand(100000)}",
    :block_size => 32,
    :inline_suffix_size => 8,
    :default_analyzer => nil,
  }
  def initialize(options = {})
    options             = DEFAULT_OPTIONS.merge(options)
    @path               = options[:path]
    @suffixes           = []
    @block_size         = options[:block_size]
    @inline_suffix_size = options[:inline_suffix_size]
    @finished           = false
    initialize_in_memory_buffer
  end

  def merge(suffix_array_reader)
    unless @suffixes.empty?
      raise "Cannot merge if the destination SuffixArrayWriter isn't empty!"
    end
    suffix_array_reader.dump_data do |partial_sarray|
      @suffixes.concat partial_sarray
    end
  end

  def add_suffixes(analyzer, data, offset)
    #analyzer.new.find_suffixes(data).each{|x| @suffixes << offset + x}
    analyzer.append_suffixes(@suffixes, data, offset)
  end

  def finish!(fulltext)
    return if @finished
    if $DEBUG
      $DEBUGF.puts "Ftsz: #{fulltext.size}"
      $DEBUGF.puts "Suffixes: #{@suffixes.size}"
      t0 = Time.new
    end
    sort!(fulltext)
    if $DEBUG
      $DEBUGF.puts "Sorted in #{Time.new - t0}"
    end
    if $DEBUG
      $DEBUGF.puts "Dumping suffixes"
      t0 = Time.new
    end
    dump_suffixes(fulltext)
    if $DEBUG
      $DEBUGF.puts "Dumped in #{Time.new - t0}"
    end
    @finished = true
  end

  private
  def dump_suffixes(fulltext)
    io = @path ? File.open(@path, "wb") : @memory_io
    io.write([@suffixes.size, @block_size || 0, @inline_suffix_size].pack("VVV"))
    if @block_size
      dump_inline_suffixes(io, fulltext)
    end
    add_padding(io)
    dump_suffix_array(io)
  ensure
    io.close if @path
  end

  def dump_inline_suffixes(io, fulltext)
    0.step(@suffixes.size, @block_size) do |suffix_idx|
      io.write([fulltext[@suffixes[suffix_idx], @inline_suffix_size]].pack("a#{@inline_suffix_size}"))
    end
  end

  def dump_suffix_array(io)
    @suffixes.each_slice(1024){|suffixes| io.write(suffixes.pack("V*")) }
  end

  def add_padding(io)
    # padding to 16-byte
    if (mod = io.pos & 0xf) != 0
      io.write("\0" * (16 - mod))
    end
  end

  def sort!(fulltext)
#    tsize = fulltext.size
    tsize = fulltext.bytesize
    if $DEBUG
      $DEBUGF.puts "Sort! #{tsize}"
      $DEBUGF.puts "Suffixes cnt: #{@suffixes.size}"
      $DEBUGF.flush
    end
    @suffixes = @suffixes.sort_by{|offset| fulltext[offset, tsize - offset]}
  end
end

end  # FTSearch
