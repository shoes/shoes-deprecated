require 'make/make'

include FileUtils

class MakeLinux
  extend Make

  class << self
    def copy_deps_to_dist
      dlls = [RUBY_SO]
      dlls += IO.readlines("make/mingw/dlls")
      dlls += %w{libvlc} if ENV['VIDEO']
      dlls.each{|dll| cp "#{EXT_RUBY}/bin/#{dll}.dll", "dist/"}
      cp "dist/zlib1.dll", "dist/zlib.dll"
      Dir.glob("../deps_cairo*/*"){|file| cp file, "dist/"}
      sh "strip -x dist/*.dll" unless ENV['DEBUG']
    end
    
    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end
  end
end
