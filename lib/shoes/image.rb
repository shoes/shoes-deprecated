class Shoes
  def image path, opts = {}, &blk
    uri = URI(path) unless uri.is_a? URI
    case uri
    when URI::HTTP, URI::FTP
      uri.open do |fin|
        name = uri.host + "." + fin.meta['etag'].gsub(/[^\w\-\.]/, '') +
          File.extname(uri.path)
        path = File.join(CACHE_DIR, name)
        unless File.exists? path
          File.open(path, 'wb') do |fout|
            while chunk = fin.read(16384)
              fout.write chunk
            end
          end
        end
      end
    end
    image_file path, opts, &blk
  end
end
