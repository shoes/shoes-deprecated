require 'make/make'

include FileUtils

class MakeDarwin
  extend Make

  class << self
    def copy_deps_to_dist
      if ENV['SHOES_DEPS_PATH']
        dylibs = IO.readlines("make/darwin/dylibs.shoes")
        if ENV['VIDEO']
          dylibs += IO.readlines("make/darwin/dylibs.video")
        end
        dylibs.each do |libn|
          cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
        end.each do |libn|
          next unless libn =~ %r!^lib/(.+?\.dylib)$!
          libf = $1
          sh "install_name_tool -id /tmp/dep/#{libn} dist/#{libf}"
          ["dist/#{NAME}-bin", *Dir['dist/*.dylib']].each do |lib2|
            sh "install_name_tool -change /tmp/dep/#{libn} @executable_path/#{libf} #{lib2}"
          end
        end
        if ENV['VIDEO']
          mkdir_p "dist/plugins"
          sh "cp -r deps/lib/vlc/**/*.dylib dist/plugins"
          sh "strip -x dist/*.dylib"
          sh "strip -x dist/plugins/*.dylib"
          sh "strip -x dist/ruby/lib/**/*.bundle"
        end
      end
    end

    def setup_system_resources
      rm_rf "#{APPNAME}.app"
      mkdir "#{APPNAME}.app"
      mkdir "#{APPNAME}.app/Contents"
      cp_r "dist", "#{APPNAME}.app/Contents/MacOS"
      mkdir "#{APPNAME}.app/Contents/Resources"
      mkdir "#{APPNAME}.app/Contents/Resources/English.lproj"
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/App.icns\""
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/Contents/Resources/App.icns\""
      rewrite "platform/mac/Info.plist", "#{APPNAME}.app/Contents/Info.plist"
      cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
      rewrite "platform/mac/pangorc", "#{APPNAME}.app/Contents/MacOS/pangorc"
      cp "platform/mac/command-manual.rb", "#{APPNAME}.app/Contents/MacOS/"
      rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
      rewrite "platform/mac/shoes", "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
      `echo -n 'APPL????' > "#{APPNAME}.app/Contents/PkgInfo"`
    end
  end
end
