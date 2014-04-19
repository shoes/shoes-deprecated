require 'digest/sha1'

class Shoes
  def self.image_temp_path uri, uext
    File.join(Dir::tmpdir, "#{uri.host}-#{Time.now.usec}" + uext)
  end
  def self.image_cache_path hash, ext
    dir = File.join(CACHE_DIR, hash[0,2])
    Dir.mkdir(dir) unless File.exists?(dir)
    File.join(dir, hash[2..-1]) + ext.downcase
  end
  def snapshot(options = {}, &block)
    options[:format]   ||= :svg

    options[:filename] ||= ( tf_path = ( require 'tempfile'
                 tf = Tempfile.new(File.basename(__FILE__)).path ))

    _snapshot options do
      block.call
    end
    return File.read(options[:filename])
  ensure
    File.unlink(tf_path) if tf_path
  end
  
  # Synchronous download - no threading. called from the bowels of
  # image.c -> rbload.c -> here. Returns
  # something that rbload.c can deal with
  class  ImgResp
     attr_accessor :headers, :body, :status
     def initalize
       @headers = {}
       @body = ''
       @status = 404
       @size = 0
     end
  end
  
  def self.image_download_sync url, opts
    puts "image_download_sync called"
    require 'open-uri'
    tmpf = File.open(opts[:save],'wb')
    result = ImgResp.new
    open url do |f|
      # everything has been downloaded at this point.
      # f is a tempfile like creature
      result.status = f.status[0] # 200, 404, etc
      result.size = f.size
      result.body = f.read
      tmpf.write(result.body)
      result.headers = f.meta
      tmpf.close
    end
    puts "image_download_sync finished"
    return result
  end
 end
