require 'ftsearch/fragment_writer'
require 'ftsearch/analysis/simple_identifier_analyzer'
require 'ftsearchrt'

class Shoes::Search
  include FTSearch
  attr_reader :index
  def initialize fields = [:uri, :body]
    field_infos = FTSearch::FieldInfos.new
    fields.each do |name|
      field_infos.add_field :name => name,
        :analyzer => FTSearch::Analysis::SimpleIdentifierAnalyzer.new
    end
    @index = FTSearch::FragmentWriter.new :path => nil, :field_infos => field_infos
  end
  def add_document hsh
    @index.add_document hsh
  end
  def finish!
    @index.finish!

    @ft = FulltextReader.new :io => StringIO.new(@index.fulltext_writer.data)
    @sa = SuffixArrayReader.new @ft, nil, :io => StringIO.new(@index.suffix_array_writer.data)
    @dm = DocumentMapReader.new :io => StringIO.new(@index.doc_map_writer.data)
  end
  def find_all terms, show = 20, prob_sort = false
    h = Hash.new{|h,k| h[k] = 0}
    weights = Hash.new(1.0)
    weights[0] = 10000000   # :uri
    weights[1] = 10000000  # :body
    hits = @sa.find_all terms
    size = hits.size
    if prob_sort && size > 10000
      iterations = 50 * Math.sqrt(size)
      offsets = @sa.lazyhits_to_offsets(hits)
      weight_arr = weights.sort_by{|id,w| id}.map{|_,v| v}
      sorted = @dm.rank_offsets_probabilistic(offsets, weight_arr, iterations)
    else
      offsets = @sa.lazyhits_to_offsets(hits)
      sorted = @dm.rank_offsets(offsets, weights.sort_by{|id,w| id}.map{|_,v| v})
    end
    sorted[0..show].map do |doc_id, count|
      [@dm.document_id_to_uri(doc_id), count]
    end
  end
end
