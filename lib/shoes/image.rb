require 'digest/sha1'

class Shoes
  EXPIRE_SEC = 60 * 60

  def image_cache_path hash, ext
    dir = File.join(CACHE_DIR, hash[0,2])
    Dir.mkdir(dir) unless File.exists?(dir)
    File.join(dir, hash[2..-1]) + ext.downcase
  end
  def image *args, &blk
    case args[0] when Integer, Hash, NilClass
      image_file *args, &blk 
    else
      path, opts = args
      opts ||= {}

      uri = (URI(path) rescue nil) unless uri.is_a? URI
      realpath = path
      case uri
      when URI::HTTP, URI::FTP
        realpath = nil
        cache = DATABASE.check_cache_for path
        uopts, uext = [], File.extname(uri.path)
        if cache
          if Time.now - cache[:saved] < EXPIRE_SEC
            realpath = image_cache_path cache[:hash], uext
            realpath = nil unless File.exists?(realpath)
          else
            uopts = [{"If-None-Match" => cache[:etag]}]
          end
        end

        unless realpath
          tmppath = File.join(Dir::tmpdir, "#{uri.host}-#{Time.now.to_i}" + uext)
          digest = Digest::SHA1.new
          begin
            uri.open(*uopts) do |fin|
              File.open(tmppath, 'wb') do |fout|
                while chunk = fin.read(16384)
                  digest << chunk
                  fout.write chunk
                end
              end

              hash = digest.hexdigest
              DATABASE.notify_cache_of path, fin.meta['etag'], digest.hexdigest
              realpath = image_cache_path hash, uext
              FileUtils.mv tmppath, realpath
            end
          rescue OpenURI::HTTPError => e
            raise e unless cache
            DATABASE.notify_cache_of path, cache[:etag], cache[:hash]
            realpath = image_cache_path cache[:hash], uext
          end
        end
      end
      image_file path, File.expand_path(realpath), opts, &blk
    end
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
end
