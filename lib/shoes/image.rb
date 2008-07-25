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
end
