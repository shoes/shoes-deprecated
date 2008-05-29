# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'strscan'
require 'ftsearch/analysis/analyzer'

module FTSearch
module Analysis
  class WhiteSpaceAnalyzer < Analyzer
    def append_suffixes(array, text, offset)
      sc = StringScanner.new(text)
      sc.skip(/(\s|\n)*/)
      until sc.eos?
        array << (sc.pos + offset)
        break unless sc.skip(/\S+\s*/)
      end

      array
    end
  end
end #  Analyzer
end  # FTSearch
