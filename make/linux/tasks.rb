module Make
  include FileUtils

  # may have symlinks for shoes.rb and shoes/, remove them and
  # may not be called for.
  def copy_files_to_dist
    #cp   "lib/shoes.rb", "dist/lib"
    #cp_r "lib/shoes", "dist/lib"
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

  # Set up symlinks to lib/shoes and lib/shoes.rb so that they
  # can be edited and tested without a rake clean/build every time we 
  # change a lib/shoes/*.rb  
  # They'll be copied (not linked) when rake install occurs. Be very
  # careful. Only Link to FILES, not to directories. Fileutils.ln_s may
  # not be the same as linux ln -s. 
  def pre_build
    puts "Prebuild: #{pwd}"
    mkdir_p "dist/lib/shoes"
    Dir.chdir "dist/lib/shoes" do
      Dir["../../../lib/shoes/*.rb"].each do |f|
        #puts "SymLinking #{f}"
        ln_s f, "." unless File.symlink? File.basename(f)
      end
    end
    Dir.chdir "dist/lib" do
      ln_s "../../lib/shoes.rb" , "shoes.rb" unless File.symlink? "shoes.rb"
      # link to exerb
      ln_s "../../lib/exerb", "exerb" unless File.symlink? "exerb"
    end
    cp_r  "fonts", "dist/fonts"
    cp_r  "samples", "dist/samples"
    Dir.chdir "dist" do
      ln_s  "../static",  "." unless File.symlink? 'static'
    end
    #cp_r  "static", "dist/static"
    cp    "README.md", "dist/README.txt"
    cp    "CHANGELOG", "dist/CHANGELOG.txt"
    cp    "COPYING", "dist/COPYING.txt"
  end

  #  Build the extensions, gems and copy to Shoes directories
  def common_build
    puts "common_build dir=#{pwd} #{SHOES_RUBY_ARCH}"
    mkdir_p "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}"
    #cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "#{TGT_DIR}/ruby/lib"
    %w[req/ftsearch/lib/* req/rake/lib/*].each do |rdir|
      FileList[rdir].each { |rlib| cp_r rlib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}" }
    end
    %w[req/ftsearch/ext/ftsearchrt req/chipmunk/ext/chipmunk].
    #%w[req/binject/ext/binject_c req/ftsearch/ext/ftsearchrt req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
      each { |xdir| copy_ext xdir, "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}" }

    gdir = "#{TGT_DIR}/lib/ruby/gems/#{RUBY_V}"
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

    def find_and_copy thelib, newplace
        testpath = ["/usr/lib/#{thelib}", "/usr/lib/x86_64-linux-gnu/#{thelib}",
          "/usr/lib/i386-linux-gnu/#{thelib}"]
        testpath.each do |tp|
          if File.exists? tp
            cp tp, newplace
            return
          end
        end
    end

    # does nothing
    def copy_deps_to_dist
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end
 
    def make_app(name)
      # name is dist/shoes
      rm_f name
      sh "#{CC} -o #{name} bin/main.o dist/shoes.a #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
      # remove the static lib
      sh "rm -f #{TGT_DIR}/shoes.a"
   end

    # make a static library 
    def make_so(name)
      puts "make_so: #{name}"
      name = 'dist/shoes.a'
      sh "ar rc #{name} #{OBJ.join(' ')}"
      sh "ranlib #{name}"
    end

    def make_installer
      if !File.exists? "crosscompile" 
        puts "Sorry, you can't create a binary installer for your Shoes"
        puts "We can't know where their ruby is installed or how it's" 
        puts "configured. Rubyies newer than 1.9.1 can hardcode installation information"
        puts "about where they are installed so they can't be copied to"
        puts "other systems which is what Shoes does. If your desire is strong"
        puts "you can set up a cross compiling arrangement. See the notes/"
        puts "folder"
      end
      #mkdir_p "pkg"
      #sh "makeself dist pkg/#{PKG}.run '#{APPNAME}' ./#{NAME}"
    end
    
    def make_userinstall
      user = ENV['USER']
      home = ENV['HOME']
      hdir = "#{home}/.shoes/#{RELEASE_NAME}"
      if File.exists? hdir
        # so we can install many times to test installing many times
        puts "Removing old lib"
        rm_r hdir
      end
      mkdir_p hdir
      cp_r  "fonts", "#{hdir}/fonts"
      cp_r  "samples", "#{hdir}/samples"
      cp_r  "static", "#{hdir}/static"
      cp    "README.md", "#{hdir}/README.txt"
      cp    "CHANGELOG", "#{hdir}/CHANGELOG.txt"
      cp    "COPYING", "#{hdir}/COPYING.txt"
      cp "dist/shoes" , "#{hdir}"
      cp "Shoes.desktop",  "#{hdir}"
      mkdir_p "#{hdir}/lib"
      sh "cp -r dist/lib/ruby #{hdir}/lib"
      # bit of a hack here. Don't copy symlinks in dist/lib
      sh "cp -r lib/shoes #{hdir}/lib"
      sh "cp -r lib/shoes.rb #{hdir}/lib/"
      sh "cp -r lib/exerb #{hdir}/lib"

      File.open("dist/Shoes.desktop",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes Federales\n"
        f << "Exec=#{hdir}/shoes\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon=#{hdir}/static/app-icon.png\n"
        f << "Categories=Application;Development;Education;\n"
      end
      sh "xdg-desktop-menu install --novendor dist/Shoes.desktop"
      puts "\n ==== NOTE: ====\n"
      puts "Shoes has been copied to #{hdir} and menus installed" 
      puts "Modify and copy the 'Shoes.desktop' if needed."
    end
  end
end
