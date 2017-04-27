module Make
  include FileUtils

  #TODO: this is probably unused 
  def copy_files_to_dist
    puts "copy_files_to_dist dir=#{pwd}"
    if ENV['APP']
      if APP['clone']
        sh APP['clone'].gsub(/^git /, "#{GIT} --git-dir=#{ENV['APP']}/.git ")
      else
        cp_r ENV['APP'], "#{TGT_DIR}/app"
      end
      if APP['ignore']
        APP['ignore'].each do |nn|
          rm_rf "dist/app/#{nn}"
        end
      end
    end

    cp_r "fonts", "#{TGT_DIR}/fonts"
    cp   "lib/shoes.rb", "#{TGT_DIR}/lib"
    cp_r "lib/shoes", "#{TGT_DIR}/lib"
    cp_r "lib/exerb", "#{TGT_DIR}/lib"
    cp_r "samples", "#{TGT_DIR}/samples"
    cp_r "static", "#{TGT_DIR}/static"
    cp   "README.md", "#{TGT_DIR}/README.txt"
    cp   "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp   "COPYING", "#{TGT_DIR}/COPYING.txt"
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

  # copy everything from the cross compiled Ruby libs as ruby wants it
  # copy any deps libraries that might ever be used in linking or when
  # building a static ar. 
  def pre_build
    puts "pre_build dir=#{`pwd`}"
    rbvt = RUBY_V
    rbvm = RUBY_V[/^\d+\.\d+/]
    mkdir_p "#{TGT_DIR}/lib"
    # clean out leftovers from last build
    rm_f "#{TGT_DIR}/libruby.so" if File.exist? "#{TGT_DIR}/libruby.so"
    rm_f "#{TGT_DIR}/libruby.so.#{rbvm}" if File.exist? "#{TGT_DIR}/libruby.so.#{rbvm}"
    rm_f "#{TGT_DIR}/libruby.so.#{rbvt}" if File.exist? "#{TGT_DIR}/libruby.so.#{rbvt}"
    cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
    # copy and link libruby.so
    cp "#{EXT_RUBY}/lib/libruby.so.#{rbvm}", "#{TGT_DIR}"
    # copy include files - it might help build gems
    mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
    cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
    chdir TGT_DIR do
      ln_s "libruby.so.#{rbvm}", "libruby.so"
    end   
    SOLOCS.each_value do |path|
      cp "#{path}", "#{TGT_DIR}"
    end
 end

  # common_build is a misnomer. Build extentions, gems
  def common_build
    copy_gems
  end

end


include FileUtils

class MakeLinux
  extend Make

  class << self
 
    def copy_deps_to_dist
      puts "copy_deps_to_dist dir=#{pwd}"
      unless ENV['GDB']
        sh    "strip -x #{TGT_DIR}/*.so.*"
        sh    "strip -x #{TGT_DIR}/*.so"
      end
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end
 
    # name {TGT_DIR}/shoes
    def make_app(name)
      puts "make_app dir=#{pwd} arg=#{name}"
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      sh "#{CC} -o #{bin} shoes/main.o  -L#{TGT_DIR} -lshoes #{LINUX_LIBS}"
      rewrite "platform/nix/shoes.launch", name, %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      chmod 0755, "#{name}" 
      # write a gdb launched shoes
      rewrite "platform/nix/shoes.launch", "#{TGT_DIR}/debug", %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH gdb $APPPATH/#{File.basename(bin)} "$@"' >> #{TGT_DIR}/debug}
      chmod 0755, "#{TGT_DIR}/debug" 
    end

    def make_so(name)
      puts "make_so dir=#{pwd} arg=#{name}"
      #ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    # make a .install with all the bits and peices. 
    def make_installer
      gtkv = APP['GTK']== 'gtk+-3.0' ? '3' : '2'
      arch = 'x86_64'
      appname =  "#{APP['name'].downcase}"
      rlname = "#{appname}-#{APP['VERSION']}-gtk#{gtkv}-#{arch}"
      #puts "Creating Pkg for #{rlname}"
      rm_r "pkg/#{rlname}" if File.exists? "pkg/#{rlname}"
      cp_r "VERSION.txt", "#{TGT_DIR}"
      mkdir_p "pkg/#{rlname}"
      sh "cp -r #{TGT_DIR}/* pkg/#{rlname}"
      Dir.chdir "pkg/#{rlname}" do
        make_desktop 
        make_uninstall_script
        make_install_script
        make_smaller unless ENV['GDB']
      end
      Dir.chdir "pkg" do
        puts `pwd`
        sh "makeself #{rlname} #{rlname}.install #{appname} \
./shoes-install.sh "
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
    
    # the install script that runs on the user's system can be simple. 
    # Copy things from where it's run to ~/.shoes/federales/ and then
    # sed the desktop file and copy it with xdg-desktop-menu
    def make_install_script
      File.open("shoes-install.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "ddir=$HOME/.shoes/#{APP['NAME']}\n"
        f << "#echo $ddir\n"
        f << "mkdir -p $ddir\n"
        f << "cp -r * $ddir/\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.desktop.tmpl >Shoes.desktop\n"
        f << "cp Shoes.desktop $ddir/Shoes.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.desktop\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.remove.tmpl >Shoes.remove.desktop\n"
        f << "cp Shoes.remove.desktop $ddir/Shoes.remove.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.remove.desktop\n"
        f << "echo \"Shoes has been copied to $ddir. and menus created\"\n"
        f << "echo \"If you don't see Shoes in the menu, logout and login\"\n"
      end
      chmod "+x", "shoes-install.sh"
    end
    
    # run strip on the libraries, remove unneeded ruby code (tk,
    #  readline and more)
    def make_smaller
      puts "Shrinking #{`pwd`}"
      sh "strip *.so"
      sh "strip *.so.*"
      Dir.glob("lib/ruby/**/*.so").each {|lib| sh "strip #{lib}"}
    end
  end
end
