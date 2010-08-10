require 'make/make'

include FileUtils

class MakeLinux
  extend Make

  class << self
    def copy_deps_to_dist
      cp    "#{::EXT_RUBY}/lib/lib#{::RUBY_SO}.so", "dist/lib#{::RUBY_SO}.so"
      ln_s  "lib#{::RUBY_SO}.so", "dist/lib#{::RUBY_SO}.so.#{::RUBY_V[/^\d+\.\d+/]}"
      cp    "/usr/lib/libgif.so", "dist/libgif.so.4"
      ln_s  "libgif.so.4", "dist/libungif.so.4"
      cp    "/usr/lib/libjpeg.so", "dist/libjpeg.so.8"
      cp    "/usr/lib/libcurl.so", "dist/libcurl.so.4"
      cp    "/usr/lib/libportaudio.so", "dist/libportaudio.so.2"
      cp    "/usr/lib/libsqlite3.so", "dist/libsqlite3.so.0"

      if ENV['VIDEO']
        cp    "/usr/lib/libvlc.so", "dist"
        ln_s  "libvlc.so", "dist/libvlc.so.0"
      end

      sh    "strip -x dist/*.so.*"
      sh    "strip -x dist/*.so"
    end
    
    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end
  end
end
