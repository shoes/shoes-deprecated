
# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'strscan'
require 'ftsearch/analysis/analyzer'

module FTSearch
module Analysis

class SimpleIdentifierAnalyzer < Analyzer
  def append_suffixes(array, text, offset)
    sc = StringScanner.new(text)
    sc.skip(/[^A-Za-z_]+/)
    until sc.eos?
      array << (sc.pos + offset)
      break unless sc.skip(/[A-Za-z_][A-Za-z0-9_]*[^A-Za-z_]*/)
    end
  end
end

end #  Analyzer
end  # FTSearch
