# Assumes Ruby is installed with RVM and installed -C --enable-load-relative
# Ain't going to work otherwise. Well, it could but who cares and has
# that much free time?
include FileUtils
module Make
  include FileUtils

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
    cp_r  "lib", "#{TGT_DIR}"
    cp_r  "samples", "#{TGT_DIR}/samples"
    cp_r  "static", "#{TGT_DIR}/static"
    cp    "README.md", "#{TGT_DIR}/README.txt"
    cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
  end

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

  def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end

  def common_build
    mkdir_p "#{TGT_DIR}/lib/ruby"
    cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "#{TGT_DIR}/lib/ruby"
    unless ENV['STANDARD']
      %w[soap wsdl xsd].each do |libn|
       # rm_rf "dist/ruby/lib/#{libn}"
      end
    end
    %w[req/ftsearch/lib/* req/rake/lib/*].each do |rdir|
      FileList[rdir].each { |rlib| cp_r rlib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}" }
    end
    #%w[req/binject/ext/binject_c req/ftsearch/ext/ftsearchrt req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
    %w[req/ftsearch/ext/ftsearchrt req/chipmunk/ext/chipmunk].
      each { |xdir| copy_ext xdir, "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}" }

    gdir = "#{TGT_DIR}/ruby/gems/#{RUBY_V}"
    {}.each do |gemn, xdir|
    #{'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
      spec = eval(File.read("req/#{gemn}/gemspec"))
      mkdir_p "#{gdir}/specifications"
      mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
      FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
      mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
      FileList["req/#{gemn}/ext/*"].each { |elib| copy_ext elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
      cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
    end
  end

  # Check the environment
  def env(x)
    unless ENV[x]
      abort "Your #{x} environment variable is not set!"
    end
    ENV[x]
  end
end

class MakeDarwin
  extend Make

  class << self
    def copy_ext xdir, libdir
      Dir.chdir(xdir) do
        `ruby extconf.rb; make`
      end
      copy_files "#{xdir}/*.bundle", libdir
    end
    
    def pre_build
      puts "HELLO! osx pre_build #{TGT_DIR} from #{`pwd`}"
      # copy Ruby, dylib, includes - need have them in place before
      # we build exts (ftsearch). Still need some install_name 
      # dancing. 
      puts "Ruby at #{EXT_RUBY}"
      rbvt = RUBY_V
      rbvm = RUBY_V[/^\d+\.\d+/]
      mkdir_p "#{TGT_DIR}/lib"
      # clean out leftovers from last build
      rm_f "#{TGT_DIR}/libruby.dylib" if File.exist? "#{TGT_DIR}/libruby.dylib"
      rm_f "#{TGT_DIR}/libruby.#{rbvm}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvm}.dylib"
      rm_f "#{TGT_DIR}/libruby.#{rbvt}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvt}.dylib"
      cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
      # copy and link libruby.dylib
      cp "#{EXT_RUBY}/lib/libruby.#{rbvt}.dylib", "#{TGT_DIR}"
      # copy include files - it might help build gems
      mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
      cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
      # can't figure out ln -s? push pwd, cd, ln, pop
      cdir = pwd
      cd TGT_DIR
      ln_s "libruby.#{rbvt}.dylib", "libruby.dylib"
      ln_s "libruby.#{rbvt}.dylib", "libruby.#{rbvm}.dylib"
      cd cdir
      #abort "Quitting"
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

    def copy_deps_to_dist
      # Generate a list of dependencies straight from the generated files.
      # Start with dependencies of shoes-bin, and then add the dependencies
      # of those dependencies. Finally, add any oddballs that must be
      # included.
      dylibs = get_dylibs("#{TGT_DIR}/#{NAME}-bin")
      dylibs.each do |dylib|
        get_dylibs(dylib).each do |d|
          dylibs << d unless dylibs.map {|lib| File.basename(lib)}.include?(File.basename(d))
        end
      end
      dylibs << '/usr/local/etc/pango/pango.modules'
      dylibs.each do |libn|
        cp "#{libn}", "#{TGT_DIR}/"
      end.each do |libn|
        next unless libn =~ %r!/lib/(.+?\.dylib)$!
        libf = $1
        chmod 0644, "#{TGT_DIR}/#{libf}"
        # Get the actual name that the file is calling itself
        otool_lib_id = `otool -D #{TGT_DIR}/#{libf} | sed -n 2p`.chomp
        sh "install_name_tool -id @executable_path/#{libf} #{TGT_DIR}/#{libf}"
        ["#{TGT_DIR}/#{NAME}-bin", *Dir['#{TGT_DIR}/*.dylib']].each do |lib2|
          sh "install_name_tool -change #{otool_lib_id} @executable_path/#{libf} #{lib2}"
        end
      end
      copy_pango_modules_to_dist
    end

    def setup_system_resources
      puts "Enter setup_system_resources"
      tmpd = "/tmp"
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
      rewrite "platform/mac/pangorc", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/pangorc"
      cp "platform/mac/command-manual.rb", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/"
      rewrite "platform/mac/shoes-launch", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
      rewrite "platform/mac/shoes", "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod 0755, "#{tmpd}/#{APPNAME}.app/Contents/MacOS/#{NAME}"
      # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
      `echo -n 'APPL????' > "#{tmpd}/#{APPNAME}.app/Contents/PkgInfo"`
      mv "#{tmpd}/#{APPNAME}.app",  "#{TGT_DIR}"
    end

    def make_stub
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
      sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
    end

    def make_app(name)
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      sh "#{CC} -L#{TGT_DIR} -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes #{OSX_ARCH}"
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
      #%w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
      #  sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{name}"
      #end
      # 
    end
    
    def make_installer
      puts "tbz_create from #{`pwd`}"
      nfs=ENV['NFS_ALTP'] 
      mkdir_p "#{nfs}pkg"
      distfile = "#{nfs}pkg/#{PKG}#{TINYVER}-osx10.9.tbz"
      Dir.chdir("#{TGT_DIR}") do
        distname = "#{PKG}#{TINYVER}"
        sh "tar -cf #{distname}.tar #{APPNAME}.app"
        sh "bzip2 -f #{distname}.tar"
        mv "#{distname}.tar.bz2", "#{distfile}"
      end
    end 
  
    # unused - was make_installer
    def make_dmg_installer
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
