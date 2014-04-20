require 'sdbm'
data_file = File.join(LIB_DIR, "sdbm-img-cache")

DATABASE = SDBM.new(data_file)

class << DATABASE
  def check_cache_for url
    t = DATABASE[url]
    return nil unless t
    flds = t.split('|')
    hsh = {:etag => flds[0], :hash => flds[1], :saved => Time.parse(flds[2]).to_i}
    return hsh
  end
  
  def notify_cache_of url, etag, hash
    if !etag || etag == ''
      etag = 'numpty'
    end
    val = [etag, hash, Time.now].join('|')
    DATABASE[url] = val
  end
end

