include FileUtils
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
  
  def rewrite_ary before, after, reg = /\#\{(\w+\[\'\w+\'\])\}/, reg2 = '\1'
    File.open(after, 'w') do |a|
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(reg) do
            if reg2.include? '\1'
              #reg2.gsub(%r!\\1!, Object.const_get($1))
              sub = eval $1
              reg2.gsub(%r!\\1!, sub)
            else
              reg2
            end
          end
        end
      end
    end
  end
end

class MakeDarwin
  extend Make

  class << self

    def change_install_names
      puts "Entering change_install_names"
      cd "#{TGT_DIR}" do
        ["#{NAME}-bin", *Dir['*.dylib'], *Dir['lib/ruby/gems/2.1.0/**/*.bundle'], *Dir['pango/modules/*/*.so']].each do |f|
          sh "install_name_tool -id @executable_path/#{File.basename f} #{f}"
          dylibs = get_dylibs f
          dylibs.each do |dylib|
            chmod 0755, dylib if File.writable? dylib
            sh "install_name_tool -change #{dylib} @executable_path/#{File.basename dylib} #{f}"
          end
        end
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

    def copy_deps_to_dist
      puts "Entering copy_deps_to_dist #{TGT_DIR}"
      # Generate a list of dependencies straight from the generated files.
      # Start with dependencies of shoes-bin, and then add the dependencies
      # of those dependencies. Finally, add any oddballs that must be
      # included.
      dylibs = get_dylibs("#{TGT_DIR}/#{NAME}-bin")
      # add the gem's bundles.
      rbvm = RUBY_V[/^\d+\.\d+/]
      Dir["#{TGT_DIR}/lib/ruby/gems/#{rbvm}.0/gems/**/*.bundle"].each do |gb|
        puts "Bundle: #{gb}"
        dylibs.concat get_dylibs(gb)
      end
      dupes = []
      dylibs.each do |dylib|
        get_dylibs(dylib).each do |d|
          if dylibs.map {|lib| File.basename(lib)}.include?(File.basename(d))
            dupes << d
          else
            dylibs << d
          end
        end
      end

      dylibs.each do |libn|
        keyf = File.basename libn
        if @brew_hsh[keyf]
          puts "Copy: #{@brew_hsh[keyf]}"
          #cp "#{libn}", "#{TGT_DIR}/" unless File.exists? "#{TGT_DIR}/#{keyf}"
          cp @brew_hsh[keyf], "#{TGT_DIR}/" unless File.exists? "#{TGT_DIR}/#{keyf}"
          chmod 0755, "#{TGT_DIR}/#{keyf}" unless File.writable? "#{TGT_DIR}/#{keyf}"
        elsif libn =~ /\/usr\/lib/
          # There are 4 /usr/lib/ that need to be in the deps lib
          # the code below won't be triggered if they are.
          puts "Adding #{libn}"
          @brew_hsh[keyf] = libn
          #cp @brew_hsh[keyf], "#{TGT_DIR}/" unless File.exists? "#{TGT_DIR}/#{keyf}"
          cp "#{ShoesDeps}/lib/#{keyf}", "#{TGT_DIR}/" unless File.exists? "#{TGT_DIR}/#{keyf}"
          chmod 0755, "#{TGT_DIR}/#{keyf}" unless File.writable? "#{TGT_DIR}/#{keyf}"
        else
          puts "Missing #{libn}"
        end
      end
      change_install_names
      # 2015-11-22 Hack Alert librsvg2 drags in some libs that are not
      # good Shoes citizens - So after the change_install_names - remove them
      ['libresolv.9.dylib', 'libicucore.A.dylib', 'libc++.1.dylib', 'libc++abi.dylib'].each do |lib|
        rm "#{TGT_DIR}/#{lib}" if File.exists? "#{TGT_DIR}/#{lib}"
      end
    end
    
    def setup_system_resources
      # called after the gems are copied into the above setup.
      # build a hash of x.dylib > ShoesDeps/**/*.dylib
      @brew_hsh = {}
      Dir.glob("#{ShoesDeps}/lib/**/*.dylib").each do |path|
        key = File.basename(path)
        @brew_hsh[key] = path
      end
      rbvm = RUBY_V[/^\d+\.\d+/]
      # Find ruby's + gems dependent libs
      cd "#{TGT_DIR}/lib/ruby/#{rbvm}.0/#{SHOES_TGT_ARCH}" do
        bundles = *Dir['*.bundle']
        puts "Bundles #{bundles}"
        cplibs = {}
        bundles.each do |bpath|
          `otool -L #{bpath}`.split.each do |lib|
            cplibs[lib] = lib if File.extname(lib)=='.dylib'
          end
        end
        cplibs.each_key do |k|
          cppath = @brew_hsh[File.basename(k)]
          if cppath
            cp cppath, "#{TGT_DIR}"
            chmod 0755, "#{TGT_DIR}/#{File.basename k}"
            puts "Copy #{cppath}"
          else
            puts "Missing Ruby: #{k}"
          end
        end
        # -id/-change the lib
        bundles.each do |f|
          dylibs = get_dylibs f
          dylibs.each do |dylib|
            if @brew_hsh[File.basename(dylib)]
              sh "install_name_tool -change #{dylib} @executable_path/../#{File.basename dylib} #{f}"
            else
              puts "Bundle lib missing #{dylib}"
            end
          end
        end
      end
    end

    def postbuild_fix
      # if this is called the only thing that has changed is libshoes.dylib
      # and shoes-bin. This hasn't needed but might involve
      #$stderr.puts "called postbuild_fix"
=begin
      install_name_tool -id @executable_path/shoes-bin shoes-bin
      install_name_tool -change /usr/local/lib/libcairo.2.dylib @executable_path/libcairo.2.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libpangocairo-1.0.0.dylib @executable_path/libpangocairo-1.0.0.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libgif.4.dylib @executable_path/libgif.4.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libjpeg.8.dylib @executable_path/libjpeg.8.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libpango-1.0.0.dylib @executable_path/libpango-1.0.0.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libglib-2.0.0.dylib @executable_path/libglib-2.0.0.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libgobject-2.0.0.dylib @executable_path/libgobject-2.0.0.dylib shoes-bin
      install_name_tool -change /usr/local/lib/libintl.8.dylib @executable_path/libintl.8.dylib shoes-bin
      install_name_tool -change /usr/local/lib/librsvg-2.2.dylib @executable_path/librsvg-2.2.dylib shoes-bin
=end
    end
    
    def osx_create_app
      puts "Enter setup_system_resources"
      # create plist version string
      tf = File.open("VERSION.txt")
      str = tf.readline
      tf.close
      flds = str.split(' ');
      APP['plist_version_string'] = "#{flds[1]} #{flds[2]} #{flds[3]}"
      puts "plist_version_string #{APP['plist_version_string']}"
      
      tmpd = "/tmp"
      rm_rf "#{tmpd}/#{APPNAME}.app"
      mkdir "#{tmpd}/#{APPNAME}.app"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents"
      cp_r "#{TGT_DIR}", "#{tmpd}/#{APPNAME}.app/Contents/MacOS"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents/Resources"
      mkdir "#{tmpd}/#{APPNAME}.app/Contents/Resources/English.lproj"
      sh "ditto \"#{APP['icons']['osx']}\" \"#{tmpd}/#{APPNAME}.app/App.icns\""
      sh "ditto \"#{APP['icons']['osx']}\" \"#{tmpd}/#{APPNAME}.app/Contents/Resources/App.icns\""
      #rewrite "platform/mac/Info.plist", "#{tmpd}/#{APPNAME}.app/Contents/Info.plist"
      rewrite "platform/mac/Info.plist", "#{tmpd}/#{APPNAME}.app/Contents/Info.plist-1"
      rewrite_ary  "#{tmpd}/#{APPNAME}.app/Contents/Info.plist-1",
         "#{tmpd}/#{APPNAME}.app/Contents/Info.plist"
      rm "#{tmpd}/#{APPNAME}.app/Contents/Info.plist-1"
      
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
      rm_rf "#{TGT_DIR}/#{APPNAME}.app"
      mv "#{tmpd}/#{APPNAME}.app",  "#{TGT_DIR}"
      # create cshoes script /Users/ccoupe/build/mavericks/Shoes.app/Contents/MacOS
      rewrite "platform/mac/cshoes.tmpl", "cshoes"
      chmod 0755, "cshoes"
    end

    def make_stub
      sh "gcc -O -isysroot #{OSX_SDK} -framework Cocoa -o stub-osx platform/mac/stub.m -I."
    end
   
    def new_so(name)
      $stderr.puts "new__so #{name}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      # TODO  fix: gtk - needs to dig deeper vs osx
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      #$stderr.puts "objs: #{objs}"
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
    
    def new_link(name)
      $stderr.puts "new_link #{name}"
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
      sh "#{CC} -L#{TGT_DIR} -o #{bin} #{tp}/main.o #{LINUX_LIBS} -lshoes #{OSX_ARCH}"
      if File.exist? "#{tp}/zzshoesbin.done"
        postbuild_fix
      else
        copy_deps_to_dist
      end
      osx_create_app # generate plist and much copying/moving
      touch "#{tp}/zzshoesbin.done"
    end

    def make_installer
      puts "tbz_create from #{`pwd`}"
      nfs=APP['Bld_Pre']
      mkdir_p "#{nfs}pkg"
      #distfile = "#{nfs}pkg/#{PKG}#{TINYVER}-osx-10.9.tbz"
      distfile = "#{nfs}pkg/#{APPNAME}-#{APP['VERSION']}-osx-10.9.tgz"
      Dir.chdir("#{TGT_DIR}") do
        unless ENV['DEBUG']
          Dir.chdir("#{APPNAME}.app/Contents/MacOS") do
            #sh "strip -x *.dylib"
            #Dir.glob("lib/ruby/**/*.bundle").each {|lib| sh "strip -x #{lib}"}
          end
        end
        distname = "#{APPNAME}-#{APP['VERSION']}".downcase
        sh "tar -cf #{distname}.tar #{APPNAME}.app"
        sh "gzip #{distname}.tar"
        sh "mv #{distname}.tar.gz #{distfile.downcase}"
        #sh "bzip2 -f #{distname}.tar"
        #mv "#{distname}.tar.bz2", "#{distfile}"
      end
      if nfs
        # copy to /pkg
        mkdir_p "pkg"
        sh  "cp #{distfile.downcase} pkg/"
      end
      # restore tmp dir to the build
    end

    def make_smaller
      puts "Shrinking #{`pwd`}"
      sh "strip *.dylib"
      Dir.glob("lib/ruby/**/*.so").each {|lib| sh "strip #{lib}"}
    end

  end
end
