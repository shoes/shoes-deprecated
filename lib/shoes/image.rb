require 'shoes/open-uri-patch'
require 'shoes/HttpResponse'
require 'openssl'
#require 'digest/sha1'

class Shoes
  def self.image_temp_path uri, uext
    File.join(Dir::tmpdir, "#{uri.host}-#{Time.now.usec}" + uext)
  end
  
  def self.image_cache_path hash, ext
    dir = File.join(CACHE_DIR, hash[0,2])
    Dir.mkdir(dir) unless File.exists?(dir)
    File.join(dir, hash[2..-1]) + ext.downcase
  end
  
  # Synchronous download - no threading. called from the bowels of
  # image.c -> rbload.c -> here. Returns
  # something that rbload.c can deal with
  # assumes HttpResponse from download.rb has been require'd
  # and with magic of duck typing, it looks like cResponse. 
  def self.image_download_sync url, opts
    # puts "image_download_sync called"
    #require 'open-uri'
    tmpf = File.open(opts[:save],'wb')
    result = HttpResponse.new
    begin
      uri_opts = {}
      uri_opts[:redirect_to_https] = true
      uri_opts[:ssl_verify_mode] = OpenSSL::SSL::VERIFY_NONE
      open url, uri_opts do |f|
        # everything has been downloaded at this point.
        # f is a tempfile like creature
        result.status = f.status[0].to_i # 200, 404, etc
        result.body = f.read
        tmpf.write(result.body)
        result.headers = f.meta
        tmpf.close
       end
    rescue => e
      raise "Image download failed for #{url} because: #{e}"
    end
    puts "image_download_sync finished"
    return result
  end
  
#  def snapshot(options = {}, &block)
#    options[:format]   ||= :svg
#
#    options[:filename] ||= ( tf_path = ( require 'tempfile'
#                 tf = Tempfile.new(File.basename(__FILE__)).path ))
#
#    _snapshot options do
#      block.call
#    end
#    return File.read(options[:filename])
#  ensure
#    File.unlink(tf_path) if tf_path
#  end
 end
 
 class Shoes::Types::App

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
end

