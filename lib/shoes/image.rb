require 'digest/sha1'

class Shoes
  def image path, opts = {}, &blk
    uri = (URI(path) rescue nil) unless uri.is_a? URI
    realpath = path
    case uri
    when URI::HTTP, URI::FTP
      hash = Digest::SHA1.hexdigest(path)
      realpath = File.join(CACHE_DIR, uri.host + "-" + hash + File.extname(uri.path))
      unless File.exists? realpath
        uri.open do |fin|
          File.open(realpath, 'wb') do |fout|
            while chunk = fin.read(16384)
              fout.write chunk
            end
          end
        end
      end
    end
    image_file path, realpath, opts, &blk
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
