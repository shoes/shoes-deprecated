require File.expand_path('make/make')

include FileUtils

class MakeXMinGW
  extend Make

  class << self
    # Execute shell calls through bash if we are compiling with mingw. This breaks us
    # out of the windows command shell if we are compiling from there.
    #def sh(*args)
    #  cmd = args.join(' ')
    #  super "bash.exe --login -i -c \"#{cmd}\""
    #end

    # a lot happens here. Ruby is called to build Makefiles
    # and the ext is compiled and then copied into Shoes libdir
    # I attempt to get the makefiles created using the cross compiler
    # values by copying rbconfig.rb and deleteing it later.
    # also I'm going to use a separate xextconf.rb. The extra arg is
    # the cross environment's headers directory.
    def copy_ext xdir, libdir
      cp "#{EXT_RUBY}/rbconfig.rb", "#{xdir}"
      Dir.chdir(xdir) do
        sh "ruby -I. xextconf.rb #{XDEPABS}; make"
      end
      rm "#{xdir}/rbconfig.rb"
      copy_files "#{xdir}/*.so", libdir
    end

    def copy_deps_to_dist
      # First copy that msvcrt-ruby191.dll from where it lives
      # XDEPABS/bin .PITA
      dlls = ["msvcrt-ruby191"]
      dlls += IO.readlines("make/xmingw/dlls").map{|dll| dll.chomp}
      dlls.each{|dll| cp "#{XDEPABS}/bin/#{dll}.dll", "dist/"}
      cp "dist/zlib1.dll", "dist/zlib.dll"
      Dir.glob("../deps_cairo*/*"){|file| cp file, "dist/"}
      sh "strip -x dist/*.dll" unless ENV['DEBUG']
    end
    
    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end

    def make_resource(t)
      sh "#{XTOOLS}windres -I. #{t.source} #{t.name}"
    end

    def make_app(name)
      bin = name+'.exe'
      rm_f bin
      #sh "#{CC} -Ldist -o #{bin} bin/main.o shoes/appwin32.o #{LINUX_LIBS} -lshoes #{Config::CONFIG['LDFLAGS']} -mwindows"
      sh "#{CC} -Ldist -o #{bin} bin/main.o shoes/appwin32.o -L#{XLIB} #{LINUX_LIBS} -lshoes  -mwindows"
      #rewrite "platform/nix/shoes.launch", name, %r!/shoes!, "/#{NAME}"
      #sh %{echo 'cd "$OLDPWD"'}
      #sh %{echo 'LD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      #chmod 0755, name
      cp "platform/msw/shoes.exe.manifest", "dist/#{NAME}.exe.manifest"
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def make_installer
      # assumes you have NSIS installed on your box in the system PATH
      def sh(*args); super; end
      mkdir_p "pkg"
      rm_rf "dist/nsis"
      cp_r  "platform/msw", "dist/nsis"
      cp APP['icons']['win32'], "dist/nsis/setup.ico"
      rewrite "dist/nsis/base.nsi", "dist/nsis/#{NAME}.nsi"
      Dir.chdir("dist/nsis") do
        #sh "\"#{env('NSIS')}\\makensis.exe\" #{NAME}.nsi"
        sh "makensis #{NAME}.nsi"
      end
      mv "dist/nsis/#{PKG}.exe", "pkg"
    end

    # override make/make.rb
    def common_build
      # copy the Ruby standard libs (like complex.rb, time.rb)
      mkdir_p "dist/ruby"
      cp_r  "#{EXT_RUBY}/lib/", "dist/ruby/lib"
      unless ENV['STANDARD']
        %w[soap wsdl xsd].each do |libn|
          rm_rf "dist/ruby/lib/#{libn}"
        end
      end
      # copy the ftsearch and rake Ruby code in Shoes to /dist/ruby/lib
      %w[req/ftsearch/lib/* req/rake/lib/*].each do |rdir|
        FileList[rdir].each { |rlib| cp_r rlib, "dist/ruby/lib" }
      end
      mkdir_p "dist/ruby/lib/mingw"
      # Now compile the extenstions that have C code
      %w[req/binject/ext/binject_c req/ftsearch/ext/ftsearchrt req/chipmunk/ext/chipmunk].
        each { |xdir| copy_ext xdir, "dist/ruby/lib/mingw" }
      # Now we have the Gems that Shoes includes
      # NOTE! If the gem (not an extension has a .so/dll/dyld library 
      # then that shared lib has to be copied into dist/shoes/ Think Sqlite3!
      gdir = "dist/ruby/gems/#{RUBY_V}"
      {'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
        spec = eval(File.read("req/#{gemn}/gemspec"))
        mkdir_p "#{gdir}/specifications"
        mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
        FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
        mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
        FileList["req/#{gemn}/ext/*"].each { |elib| copy_ext elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
        cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
      end
    end
  end
end
