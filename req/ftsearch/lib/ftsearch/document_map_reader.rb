# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

module FTSearch

class DocumentMapReader
  DEFAULT_OPTIONS = {
    :path => nil,
    :io   => nil,
  }
  def initialize(options = {})
    options = DEFAULT_OPTIONS.merge(options)
    unless options[:path] || options[:io]
      raise ArgumentError, "Need either the path to the suffix array file or an IO."
    end
    if options[:path]
      io = File.open(options[:path], "rb")
    else
      io = options[:io]
    end
    @uri_tbl, @field_arr = Marshal.load(io)
  end

  def document_id(suffix_idx, offset)
    idx = binary_search(@field_arr, offset, 0, @field_arr.size)
    @field_arr[idx][1]
  end

  def field_id(suffix_idx, offset)
    idx = binary_search(@field_arr, offset, 0, @field_arr.size)
    @field_arr[idx][2]
  end

  def field_size(suffix_idx, offset)
    idx = binary_search(@field_arr, offset, 0, @field_arr.size)
    @field_arr[idx][3]
  end

  def field_info(suffix_idx, offset)
    idx = binary_search(@field_arr, offset, 0, @field_arr.size)
    @field_arr[idx]
  end

  def document_uri(suffix_idx, offset)
    doc_id = document_id(suffix_idx, offset)
    @uri_tbl[doc_id]
  end

  def document_id_to_uri(doc_id)
    @uri_tbl[doc_id]
  end

  def offsets_to_field_infos(offsets)
    offsets.map{|off| @field_arr[binary_search(@field_arr, off, 0, @field_arr.size)]}
  end

  def rank_offsets(offsets, weights)
    h = Hash.new{|h,k| h[k] = 0.0}
    offsets_to_field_infos(offsets).each do |offset, doc_id, field_id, field_size|
      h[doc_id] += weights[field_id] / field_size
    end
    sort_score_hash(h)
  end

  def rank_offsets_probabilistic(offsets, weights, iterations)
    h     = Hash.new{|h,k| h[k] = 0.0}
    infos = offsets_to_field_infos(offsets)
    max   = infos.size
    while iterations > 0
      offset, doc_id, field_id, field_size = infos[rand(max)]
      h[doc_id] += weights[field_id] / field_size
      iterations -= 1
    end
    sort_score_hash(h)
  end

  def dump_data
    [@uri_tbl, @field_arr]
  end

  def num_documents
    @uri_tbl.size
  end

  private
  def sort_score_hash(h)
    h.sort_by{|_,score| -score}
  end

  def binary_search(arr, offset, from, to)
    middle = 0
    while to - from > 1
      middle = (from + to) / 2
      pivot,  = arr[middle]
      if offset < pivot
        to = middle
      else
        from = middle
      end
    end

    from
  end

end
end  # FTSearch
