require File.expand_path('make/make')

include FileUtils

class MakeLinux
  extend Make

  class << self
    def copy_ext xdir, libdir
      Dir.chdir(xdir) do
        unless system "ruby", "extconf.rb" and system "make"
          raise "Extension build failed"
        end
      end
      copy_files "#{xdir}/*.so", libdir
    end

    def find_and_copy thelib,newplace
        testpath = ENV['LD_LIBRARY_PATH'].nil? ? [] : ENV['LD_LIBRARY_PATH'].split(/:/).map(&:strip)
        testpath += [
          "/usr/lib/#{thelib}",
          "/usr/lib64/#{thelib}",
          "/usr/lib/x86_64-linux-gnu/#{thelib}",
          "/usr/lib/i386-linux-gnu/#{thelib}"
        ]
        testpath.each do |tp|
          if File.exists? tp
            cp tp, newplace
            return
          end
        end
    end

    def copy_deps_to_dist
      cp    "#{::EXT_RUBY_LIB}/lib#{::RUBY_SO}.so", "dist/lib#{::RUBY_SO}.so"
      ln_s  "lib#{::RUBY_SO}.so", "dist/lib#{::RUBY_SO}.so.#{::RUBY_V[/^\d+\.\d+/]}"
      find_and_copy "libgif.so", "dist/libgif.so.4"
      ln_s  "libgif.so.4", "dist/libungif.so.4"
      find_and_copy "libjpeg.so", "dist/libjpeg.so.8"
      find_and_copy "libcurl.so", "dist/libcurl.so.4"
      find_and_copy "libportaudio.so", "dist/libportaudio.so.2"
      find_and_copy  "libsqlite3.so", "dist/libsqlite3.so.0"
      sh    "strip -x dist/*.so.*"
      sh    "strip -x dist/*.so"
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end

    def make_app(name)
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{RbConfig::CONFIG['LDFLAGS']}"
      rewrite "platform/nix/shoes.launch", name, %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      chmod 0755, name
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def make_installer
      mkdir_p "pkg"
      sh "makeself dist pkg/#{PKG}.run '#{APPNAME}' ./#{NAME}"
    end
  end
end
