module Make
  include FileUtils

  def cc(t)
    sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
  end

  # Subs in special variables
  def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
    File.open(after, 'w') do |a|
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(reg) do
            if reg2.include? '\1'
              reg2.gsub(%r!\\1!, Object.const_get($1))
            else
              reg2
            end
          end
        end
      end
    end
  end
end


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
    
    def copy_files_to_dist 
      if ENV['APP']
        if APP['clone']
          sh APP['clone'].gsub(/^git /, "#{GIT} --git-dir=#{ENV['APP']}/.git ")
        else
          cp_r ENV['APP'], "#{TGT_DIR}/app"
        end
        if APP['ignore']
          APP['ignore'].each do |nn|
            rm_rf "#{TGT_DIR}/app/#{nn}"
          end
        end
      end
      cp_r  "fonts", "#{TGT_DIR}/fonts"
      cp_r  "lib", "#{TGT_DIR}/lib"
      cp_r  "samples", "#{TGT_DIR}/samples"
      cp_r  "static", "#{TGT_DIR}/static"
      cp    "README.md", "#{TGT_DIR}/README.txt"
      cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
      cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
    end
    
    def pre_build
      puts "osx pre_build #{TGT_DIR} from #{`pwd`}"
    end
    
    def copy_ext_osx xdir, libdir
      Dir.chdir(xdir) do
        `ruby extconf.rb; make`
      end
      copy_files "#{xdir}/*.bundle", libdir
    end

    def common_build
      puts "Common build called"
      mkdir_p "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}"
      # FIXME: too lazy to figure out install_name_tool for this:
      cd TGT_DIR do
        dirp = RbConfig::CONFIG['libdir']
        so =   RbConfig::CONFIG['LIBRUBY_SO']
        ln_s "#{dirp}/#{so}", so unless File.exist? so
      end
      #cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "dist/ruby/lib"
      #cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "#{TGT_DIR}/lib/ruby"
      #cp "#{EXT_RUBY}/lib/libruby.dylib", "#{TGT_DIR}"
      %w[req/rake/lib/*].each do |rdir|
        #FileList[rdir].each { |rlib| cp_r rlib, "dist/ruby/lib" }
        FileList[rdir].each { |rlib| cp_r rlib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}" }
     end
     #%w[req/binject/ext/binject_c req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
     #  each { |xdir| copy_ext_osx xdir, "dist/ruby/lib/#{SHOES_RUBY_ARCH}" }
     %w[req/chipmunk/ext/chipmunk].
        each { |xdir| copy_ext_osx xdir, "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}" }

      gdir = "#{TGT_DIR}/ruby/gems/#{RUBY_V}"
      #{'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
      {}.each do |gemn, xdir|
        spec = eval(File.read("req/#{gemn}/gemspec"))
        mkdir_p "#{gdir}/specifications"
        mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
        FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
        mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
        FileList["req/#{gemn}/ext/*"].each { |elib| copy_ext_osx elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
        cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
      end
    end

    # Get a list of linked libraries for lib (discard the non-indented lines)
    def get_dylibs lib
      `otool -L #{lib}`.split("\n").inject([]) do |dylibs, line|
        if  line =~ /^\S/ or line =~ /System|@executable_path|libobjc/
          dylibs
        else
          dylibs << line.gsub(/\s\(compatibility.*$/, '').strip
        end
      end
    end
    
    def dylibs_to_change lib
      `otool -L #{lib}`.split("\n").inject([]) do |dylibs, line|
        if  line =~ /^\S/ or line =~ /System|@executable_path|libobjc/
          dylibs
        else
          dylibs << line.gsub(/\s\(compatibility.*$/, '').strip
        end
      end
    end
    
    def change_install_names 
      cd "#{TGT_DIR}" do
        ["#{NAME}-bin", "pango-querymodules", *Dir['*.dylib'], *Dir['pango/modules/*.so']].each do |f|
          sh "install_name_tool -id @executable_path/#{File.basename f} #{f}"
          dylibs = dylibs_to_change(f)
          dylibs.each do |dylib|
            # another Cecil hack
            chmod 0755, dylib if File.writable? dylib
            sh "install_name_tool -change #{dylib} @executable_path/#{File.basename dylib} #{f}"
          end
        end
      end
    end
    
    def copy_pango_modules_to_dist 
      puts "Entering copy_pango_modules_to_dist"
      modules_file = `brew --prefix`.chomp << '/etc/pango/pango.modules'
      modules_path = File.open(modules_file) {|f| f.grep(/^# ModulesPath = (.*)$/){$1}.first}
      mkdir_p "#{TGT_DIR}/pango"
      cp_r modules_path, "#{TGT_DIR}/pango"
      # Another Cecil hack ahead
      Dir.glob("#{TGT_DIR}/pango/modules/*").each do |f|
        chmod 0755, f unless File.writable? f
      end
      cp `which pango-querymodules`.chomp, "#{TGT_DIR}/"
      # another hack
      chmod 0755, "#{TGT_DIR}/pango-querymodules"
    end
    

    def copy_deps_to_dist
      puts "Entering copy_deps_to_dist"
      copy_pango_modules_to_dist
      # Generate a list of dependencies straight from the generated files.
      # Start with dependencies of shoes-bin and pango-querymodules, and then
      # add the dependencies of those dependencies.
      dylibs = dylibs_to_change("#{TGT_DIR}/#{NAME}-bin")
      dylibs.concat dylibs_to_change("#{TGT_DIR}/pango-querymodules")
      dupes = []
      dylibs.each do |dylib|
        dylibs_to_change(dylib).each do |d|
          if dylibs.map {|lib| File.basename(lib)}.include?(File.basename(d))
            dupes << d
          else
            dylibs << d
          end
        end
      end
      #dylibs.each {|libn| cp "#{libn}", "dist/" unless File.exists? "dist/#{libn}"}
      # clunky hack begins - Homebrew keg issue? ro duplicates do exist
      # make my own dups hash - not the same as dupes. 
      dups = {}
      dylibs.each do |libn| 
        keyf = File.basename libn
        if !dups[keyf] 
          cp "#{libn}", "#{TGT_DIR}/" unless File.exists? "#{TGT_DIR}/#{keyf}"
          dups[keyf] = true
          chmod 0755, "#{TGT_DIR}/#{keyf}" unless File.writable? "#{TGT_DIR}/#{keyf}"
        end
      end
    end

    def setup_system_resources
      tmpd = "/tmp"
      mkdir_p tmpd
      puts "Perfoming :setup_system_resources from #{`pwd`} to dir #{tmpd} "
      rm_rf "#{tmpd}/#{APPNAME}.app"
      mkdir "#{tmpd}/#{APPNAME}.app"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents"
      cp_r "#{TGT_DIR}", "#{tmpd}/#{APPNAME}.app/Contents/MacOS"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents/Resources"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents/Resources/English.lproj"
      sh "ditto \"#{APP['icons']['osx']}\" \"#{tmpd}/#{APPNAME}.app/App.icns\""
      sh "ditto \"#{APP['icons']['osx']}\" \"#{tmpd}/#{APPNAME}.app/Contents/Resources/App.icns\""
      rewrite "platform/mac/Info.plist", "#{tmpd}/#{APPNAME}.app/Contents/Info.plist"
      cp "platform/mac/version.plist", "#{tmpd}/#{APPNAME}.app/Contents/"
      #rewrite "platform/mac/pangorc", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/pangorc"
      cp "platform/mac/command-manual.rb", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/"
      rewrite "platform/mac/simple-launch", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
      rewrite "platform/mac/shoes", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}"
      #chmod_R 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/pango-querymodules"
      # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
      `echo -n 'APPL????' > "#{tmpd}/#{APPNAME}.app/Contents/PkgInfo"`
      rm_rf "#{TGT_DIR}/#{APPNAME}.app"
      #NFS=ENV['NFS_ALTP']
      mv "#{tmpd}/#{APPNAME}.app", "#{TGT_DIR}"
    end

    def make_stub
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
      sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
    end

    def make_app(name)
      puts "Entering make_app"
      bin = "#{TGT_DIR}/#{NAME}-bin"
      rm_f "#{TGT_DIR}/#{NAME}"
      rm_f bin
      sh "#{CC} -L#{TGT_DIR} -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{OSX_ARCH} -L/usr/local/lib -lgif"
    end

    def make_so(name)
      puts "Central make_so"
      name = "#{TGT_DIR}/lib#{SONAME}.#{DLEXT}"
      #ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, ""
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def make_installer
      abort "Sorry, you can't distribute a Loose Shoes. It's tightly linked to your\
configuration that the other user won't have. You can build a Tight Shoes\
which is more universal. See 'rake -T' 'rake osx:setup:mavericks' perhaps?"
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
