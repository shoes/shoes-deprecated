require File.expand_path('make/make')

include FileUtils

class MakeMinGW
  extend Make

  class << self
    # Execute shell calls through bash if we are compiling with mingw. This breaks us
    # out of the windows command shell if we are compiling from there.
    def sh(*args)
      cmd = args.join(' ')
      super "bash.exe --login -i -c \"#{cmd}\""
    end

    def copy_ext xdir, libdir
      Dir.chdir(xdir) do
        sh 'ruby extconf.rb; make'
      end
      copy_files "#{xdir}/*.so", libdir
    end

    def copy_deps_to_dist
      dlls = [RUBY_SO]
      dlls += IO.readlines("make/mingw/dlls").map{|dll| dll.chomp}
      dlls += %w{libvlc} if ENV['VIDEO']
      dlls.each{|dll| cp "#{EXT_RUBY}/bin/#{dll}.dll", "dist/"}
      cp "dist/zlib1.dll", "dist/zlib.dll"
      Dir.glob("../deps_cairo*/*"){|file| cp file, "dist/"}
      sh "strip -x dist/*.dll" unless ENV['DEBUG']
    end
    
    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end

    def make_resource(t)
      sh "windres -I. #{t.source} #{t.name}"
    end

    def make_app(name)
      bin = name
      rm_f bin
      sh "#{CC} -Ldist -o #{bin} bin/main.o shoes/appwin32.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']} -mwindows"
      rewrite "platform/nix/shoes.launch", name, %r!/shoes!, "/#{NAME}"
      sh %{echo 'cd "$OLDPWD"'}
      sh %{echo 'LD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      chmod 0755, name
      cp "platform/msw/shoes.exe.manifest", "dist/#{NAME}.exe.manifest"
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def make_installer
      def sh(*args); super; end
      mkdir_p "pkg"
      rm_rf "dist/nsis"
      cp_r  "platform/msw", "dist/nsis"
      cp APP['icons']['win32'], "dist/nsis/setup.ico"
      rewrite "dist/nsis/base.nsi", "dist/nsis/#{NAME}.nsi"
      Dir.chdir("dist/nsis") do
        sh "\"#{env('NSIS')}\\makensis.exe\" #{NAME}.nsi"
      end
      mv "dist/nsis/#{PKG}.exe", "pkg"
    end
  end
end
