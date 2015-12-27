# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'stringio'
module FTSearch

module InMemoryWriter
  def initialize_in_memory_buffer
    @memory_io = StringIO.new("")
    @memory_io.set_encoding("ASCII-8BIT")
  end

  def data
    if @path
     File.open(@path, "rb"){|f| f.read} rescue nil
    else
      $DEBUGF.puts "util:data: in @memory_io: #{@memory_io.string.size} #{@memory_io.string.bytesize}" if $DEBUG
      @memory_io.string.clone
    end
  end
end

end  # FTSearch
