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
  
  def delete_cache 
    DATABASE.each do |k, val|
      v = val.split('|')
      path = Shoes::image_cache_path v[1], File.extname(k)
      $stderr.puts "ext cache delete: #{path}"
      File.delete path if File.exist? path
    end        
    DATABASE.clear
    DATABASE.close
    # TODO: reopen - somehow.
  end
end

