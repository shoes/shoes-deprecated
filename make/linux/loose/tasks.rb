module Make
  include FileUtils

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
  
  def copy_gems
  end

  # Set up symlinks to lib/shoes and lib/shoes.rb so that they
  # can be edited and tested without a rake clean/build every time we 
  # change a lib/shoes/*.rb  
  # They'll be copied (not linked) when rake install occurs. Be very
  # careful. Only Link to FILES, not to directories. Fileutils.ln_s may
  # not be the same as linux ln -s. 
  def pre_build
    puts "Prebuild: #{pwd}"
    return
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
    %w[req/rake/lib/*].each do |rdir|
      FileList[rdir].each { |rlib| cp_r rlib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}" }
    end
    #%w[req/binject/ext/binject_c req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
    #%w[req/chipmunk/ext/chipmunk].
    # each { |xdir| copy_ext xdir, "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_RUBY_ARCH}" }

    gdir = "#{TGT_DIR}/lib/ruby/gems/#{RUBY_V}"
    # Loose shoes doesn't build gems
    {}.each do |gemn, xdir|
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

    # does nothing
    def copy_deps_to_dist
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "dist/static/app-icon.png"
    end
 
    def make_app(name)
      # name is dist/shoes
      rm_f name
      sh "#{CC} -o #{name} shoes/main.o dist/shoes.a #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
      # remove the static lib
      sh "rm -f #{TGT_DIR}/shoes.a"
    end
   
    # this is for the file task based new_builder
    def new_link
      #sh "#{CC} -o dist/shoes shoes/main.o #{SubDirs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
      # above almost works. Lets try something old like: Make  shoes.a from
      # loose obj files (re-implement make_so)

      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        $stderr.puts "collecting .o from #{d}"
        objs = objs + FileList["#{d}/*.o"]      
      end
      # TODO  fix: gtk - needs to dig deeper
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "ar -rc dist/shoes.a #{objs.join(' ')}"
      sh "ranlib dist/shoes.a"
      sh "#{CC} -o dist/shoes #{main_o} dist/shoes.a #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
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
    end
    
    def make_userinstall
      user = ENV['USER']
      home = ENV['HOME']
      hdir = "#{home}/.shoes/#{APP['NAME']}"
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
      cp    "VERSION.txt", "#{hdir}"
      cp "dist/shoes" , "#{hdir}"
      #cp "Shoes.desktop",  "#{hdir}"
      mkdir_p "#{hdir}/lib"
      sh "cp -r dist/lib/ruby #{hdir}/lib"
      # bit of a hack here. Don't copy symlinks in dist/lib
      sh "cp -r lib/shoes #{hdir}/lib"
      sh "cp -r lib/shoes.rb #{hdir}/lib/"
      sh "cp -r lib/exerb #{hdir}/lib"
      Dir.chdir hdir do
        make_desktop 
        make_uninstall_script
        make_install_script
        # run the generated install script to build menus
        sh "./shoes-install.sh"
      end
    end
     
    def make_desktop
      File.open("Shoes.desktop.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Application;Development;Education;\n"
      end
      File.open("Shoes.remove.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Uninstall Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes-uninstall.sh\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Delete Shoes\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Application;Development;Education;\n"
      end
    end
    
    def make_uninstall_script
      File.open("shoes-uninstall.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "cd $HOME/.shoes/#{APP['NAME']}\n"
        f << "xdg-desktop-menu uninstall Shoes.remove.desktop\n"
        f << "xdg-desktop-menu uninstall Shoes.desktop\n"
        f << "cd ../\n"
        f << "rm -rf #{APP['NAME']}\n"
      end
      chmod "+x", "shoes-uninstall.sh"
    end
    
    # Note: this is different from make_install script for Tight Shoes
    def make_install_script
      File.open("shoes-install.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "ddir=$HOME/.shoes/#{APP['NAME']}\n"
        f << "cd $ddir\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.desktop.tmpl >Shoes.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.desktop\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.remove.tmpl >Shoes.remove.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.remove.desktop\n"
        f << "echo \"Shoes has been copied to $ddir. and menus created\"\n"
        f << "echo \"If you don't see Shoes in the menu, logout and login\"\n"
      end
      chmod "+x", "shoes-install.sh"
    end   
    
  end
end
