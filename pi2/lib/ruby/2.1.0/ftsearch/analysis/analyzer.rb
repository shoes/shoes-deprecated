# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

module FTSearch
module Analysis

class Analyzer
  def find_suffixes(text)
    ret = []
    append_suffixes(ret, text, 0)
    ret
  end
end

end # Analysis
end # FTSearch
