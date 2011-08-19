require File.expand_path('make/make')

include FileUtils

class MakeDarwin
  extend Make

  class << self
    def copy_ext xdir, libdir
      Dir.chdir(xdir) do
        `ruby extconf.rb; make`
      end
      copy_files "#{xdir}/*.bundle", libdir
    end

    def copy_deps_to_dist
      # Generate a list of dependencies straight from the generated files

      # Treat these libs separately, since they are not managed by Homebrew
      non_brew_libs = Array.new
      non_brew_libs << {:libdir => Config::CONFIG['prefix'], :lib => 'lib/libruby.1.9.1.dylib'}
      non_brew_libs << {:libdir => '/usr', :lib => 'lib/libiconv.2.dylib'}

      non_brew_libs.each do |l|
        cp "#{l[:libdir]}/#{l[:lib]}", "dist/"
      end

      if ENV['SHOES_DEPS_PATH']
        dylibs = IO.readlines("make/darwin/dylibs.shoes").map(&:chomp)
        if ENV['VIDEO']
          dylibs += IO.readlines("make/darwin/dylibs.video").map(&:chomp)
        end
        dylibs.each do |libn|
          cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
        end
        dylibs += non_brew_libs.map { |l| l[:lib] }
        dylibs.each do |libn|
          next unless libn =~ %r!^lib/(.+?\.dylib)$!
          libf = $1
          # Get the actual name that the file is calling itself by grabbing
          # the second line of otool -D:
          otool_lib_id = `otool -D dist/#{libf} | sed -n 2p`.chomp

          # Set new id 
          sh "install_name_tool -id @executable_path/#{libf} dist/#{libf}"
          ["dist/#{NAME}-bin", *Dir['dist/*.dylib']].each do |lib2|
            sh "install_name_tool -change #{otool_lib_id} @executable_path/#{libf} #{lib2}"
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

    def make_stub
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
      sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
    end

    def make_app(name)
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes -arch x86_64"
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
      %w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
        sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{name}"
      end
    end

    def make_installer
      dmg_ds, dmg_jpg = "platform/mac/dmg_ds_store", "static/shoes-dmg.jpg"
      if APP['dmg']
        dmg_ds, dmg_jpg = APP['dmg']['ds_store'], APP['dmg']['background']
      end

      mkdir_p "pkg"
      rm_rf "dmg"
      mkdir_p "dmg"
      cp_r "#{APPNAME}.app", "dmg"
      unless ENV['APP']
        mv "dmg/#{APPNAME}.app/Contents/MacOS/samples", "dmg/samples"
      end
      ln_s "/Applications", "dmg/Applications"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-bin"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-launch"
      sh "DYLD_LIBRARY_PATH= platform/mac/pkg-dmg --target pkg/#{PKG}.dmg --source dmg --volname '#{APPNAME}' --copy #{dmg_ds}:/.DS_Store --mkdir /.background --copy #{dmg_jpg}:/.background" # --format UDRW"
      rm_rf "dmg"
    end
  end
end
