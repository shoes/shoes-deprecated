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

  def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end
  
  # a stub, loose linux doesn't copy gems but the Builder will call it
  def copy_gems  
  end

end


include FileUtils

class MakeBSD
  extend Make

  class << self

    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end
  
    # this is called from the file task based new_builder
    def new_so (name) 
      tgts = File.expand_path(name)
      tgtd = File.dirname(name)
      $stderr.puts "new_so: #{tgtd} from #{tgts}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "ar -rc #{tgtd}/shoes.lib #{objs.join(' ')}"
      sh "ranlib #{tgtd}/shoes.lib"    
    end
    
    def new_link name
      tgts = File.expand_path(name)
      tgtd = File.dirname(name)
      $stderr.puts "new_link: #{tgtd} from #{name}"
      sh "#{CC} -o #{TGT_DIR}/shoes  #{TGT_DIR}/#{APP['Bld_Tmp']}/main.o #{TGT_DIR}/shoes.lib #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
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
      cp_r  "#{TGT_DIR}/fonts", "#{hdir}/fonts"
      cp_r  "#{TGT_DIR}/samples", "#{hdir}/samples"
      cp_r  "#{TGT_DIR}/static", "#{hdir}/static"
      cp    "#{TGT_DIR}/README.md", "#{hdir}/README.txt"
      cp    "#{TGT_DIR}/CHANGELOG", "#{hdir}/CHANGELOG.txt"
      cp    "#{TGT_DIR}/COPYING", "#{hdir}/COPYING.txt"
      cp    "#{TGT_DIR}/VERSION.txt", "#{hdir}"
      cp    "#{TGT_DIR}/shoes" , "#{hdir}"
      cp    "#{TGT_DIR}/shoes.lib",hdir
      mkdir_p "#{hdir}/lib"
      #sh "cp -r #{TGT_DIR}/lib/ruby #{hdir}/lib"
      # bit of a hack here. Don't copy symlinks in dist/lib
      sh "cp -r #{TGT_DIR}/lib/shoes #{hdir}/lib"
      sh "cp -r #{TGT_DIR}/lib/shoes.rb #{hdir}/lib/"
      sh "cp -r #{TGT_DIR}/lib/exerb #{hdir}/lib"
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
        f << "#!/usr/bin/env bash\n"
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
        f << "#!/usr/bin/env bash\n"
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
