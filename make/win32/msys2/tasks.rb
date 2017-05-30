module Make
  include FileUtils

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
    sh "#{CC} -I. -c -o#{t.name} #{WINDOWS_CFLAGS} #{t.source}"
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

  #  Copy the rubyinstaller libs - sigh - don't copy all of the gems.
  #  Then copy the deps.
  def pre_build
    $stderr.puts "pre_build dir=#{`pwd`}"
    rbvt = RUBY_V
    rbvm = RUBY_V[/^\d+\.\d+/]
    # remove leftovers from previous rake.
    rm_rf "#{TGT_DIR}/lib"
    rm_rf "#{TGT_DIR}/etc"
    rm_rf "#{TGT_DIR}/share"
    rm_rf "#{TGT_DIR}/conf.d"
    mkdir_p "#{TGT_DIR}/lib/ruby"
    cp_r "#{EXT_RUBY}/lib/ruby/#{rbvt}", "#{TGT_DIR}/lib/ruby"
    # copy include files
    mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
    cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
    # copy site_ruby (gems updates)
    cp_r "#{EXT_RUBY}/lib/ruby/site_ruby", "#{TGT_DIR}/lib/ruby"
    # copy vendor_ruby
    cp_r "#{EXT_RUBY}/lib/ruby/vendor_ruby", "#{TGT_DIR}/lib/ruby"
    # make empty gem libs
    mkdir_p "#{TGT_DIR}/lib/ruby/gems/#{rbvt}"
    ['build_info', 'cache', 'doc', 'extentions', 'gems', 'specifications' ].each do
       |d| mkdir_p "#{TGT_DIR}/lib/ruby/gems/#{rbvt}/#{d}"
    end
    # copy default gemspecs
    cp_r "#{EXT_RUBY}/lib/ruby/gems/#{rbvt}/specifications/default",
      "#{TGT_DIR}/lib/ruby/gems/#{rbvt}/specifications"
    # from default gemspecs , copy what's in gems (rake, rdoc, test-unit)
    # other defaults like bigdecimal are inside ruby. Grr.
    specs = Dir.glob("#{EXT_RUBY}/lib/ruby/gems/#{rbvt}/specifications/default/*.gemspec")
    specs.each do |spec|
       dirname = File.basename(spec, ".gemspec")
       next unless File.directory?("#{EXT_RUBY}/lib/ruby/gems/#{rbvt}/gems/#{dirname}")
       #puts "Copy #{dirname}"
       cp_r "#{EXT_RUBY}/lib/ruby/gems/#{rbvt}/gems/#{dirname}", "#{TGT_DIR}/lib/ruby/gems/#{rbvt}/gems"
    end
    # copy the deplibs (see env.rb)
    SOLOCS.each_value do |path|
      cp "#{path}", "#{TGT_DIR}"
    end
    # setup GTK stuff
    mkdir_p "#{TGT_DIR}/share/glib-2.0/schemas"
    if APP['GTK'] == "gtk+-2.0"
      cp_r"#{ShoesDeps}/share/glib-2.0/schemas/gschema.dtd",
        "#{TGT_DIR}/share/glib-2.0/schemas"
      cp_r "#{ShoesDeps}/share/fontconfig", "#{TGT_DIR}/share"
      cp_r "#{ShoesDeps}/share/themes", "#{TGT_DIR}/share"
      cp_r "#{ShoesDeps}/share/xml", "#{TGT_DIR}/share"
      cp_r "#{ShoesDeps}/share/icons", "#{TGT_DIR}/share"
    elsif APP['GTK'] == "gtk+-3.0"
      cp  "#{ShoesDeps}/share/glib-2.0/schemas/gschemas.compiled" ,
        "#{TGT_DIR}/share/glib-2.0/schemas"
      cp_r "#{ShoesDeps}/share/fontconfig", "#{TGT_DIR}/share"
      cp_r "#{ShoesDeps}/share/themes", "#{TGT_DIR}/share"
      cp_r "#{ShoesDeps}/share/xml", "#{TGT_DIR}/share"
      mkdir_p "#{TGT_DIR}/share/icons"
      cp_r "#{ShoesDeps}/share/icons/hicolor", "#{TGT_DIR}/share/icons" 
    else
      cp  "#{ShoesDeps}share/glib-2.0/schemas/gschemas.compiled" ,
        "#{TGT_DIR}/share/glib-2.0/schemas"
    end
    sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
    cp_r "#{ShoesDeps}/etc", TGT_DIR
    if ENABLE_MS_THEME
      ini_path = "#{TGT_DIR}/etc/gtk-3.0"
      mkdir_p ini_path
      File.open "#{ini_path}/settings.ini", mode: 'w' do |f|
        f.write "[Settings]\n"
        f.write "gtk-theme-name=win32"
      end
    end
    mkdir_p "#{ShoesDeps}/lib"
    if APP['GTK'] == "gtk+-3.0"
      cp_r "#{ShoesDeps}/lib/gtk-3.0", "#{TGT_DIR}/lib" #  shoes, exerb, ruby here
    else
      cp_r "#{ShoesDeps}/lib/gtk-2.0", "#{TGT_DIR}/lib" #  shoes, exerb, ruby here
    end
    
    bindir = "#{GtkDeps}/bin"
    # some gdk-pixbuf versions may need to execute some gtk setup apps - sigh
    gdkcache = "#{TGT_DIR}/lib/gdk-pixbuf-2.0/2.10.0/"
    $stderr.puts "create cache #{gdkcache}"
    mkdir_p gdkcache
    Dir.chdir(gdkcache) do
      `gdk-pixbuf-query-loaders > loaders.cache`
    end
    
    #cp_r "#{bindir}/fc-cache.exe", TGT_DIR
    # newer versions of gkt3 changed the name of an exe- grrr.
    if File.exist?("#{bindir}/gtk-update-icon-cache-3.0.exe")
      cp "#{bindir}/gtk-update-icon-cache-3.0.exe",
            "#{TGT_DIR}/gtk-update-icon-cache.exe"
    else 
      cp  "#{bindir}/gtk-update-icon-cache.exe", TGT_DIR
    end
    if ENABLE_MS_THEME
      # we need to copy Adwaita icons - huge but Shoes/Gtk needs many
      $stderr.puts "Copying Adwaita icons"
      cp_r "#{ShoesDeps}/share/icons/Adwaita", "#{TGT_DIR}/share/icons"
      #$stderr.puts "Narrowing icons copy to actions"
      #mkdir_p "#{TGT_DIR}/share/icons/Adwaita"
      #icons_p = Dir.glob("#{ShoesDeps}/share/icons/Adwaita/*/actions")
      #icons_p.each do |p| 
      #  last_dir = p.split('/')[-2]
      #  mkdir_p "#{TGT_DIR}/share/icons/Adwaita/#{last_dir}"
      #  $stderr.puts "icons_p: #{p} to #{TGT_DIR}/share/icons/Adwaita/#{last_dir}"
      #  cp_r p, "#{TGT_DIR}/share/icons/Adwaita/#{last_dir}"
      #end
      # TODO: groan - upate icon cache? 
      $stderr.puts "Force icon-cache update"
      `#{TGT_DIR}/gtk-update-icon-cache.exe  -f #{TGT_DIR}/share/icons/Adwaita`
    end

    # below for debugging purposes
    if ENV['GDB'] 
      cp "#{bindir}/fc-cat.exe", TGT_DIR
      cp "#{bindir}/fc-list.exe", TGT_DIR
      cp "#{bindir}/fc-match.exe", TGT_DIR
      cp "#{bindir}/fc-pattern.exe", TGT_DIR
      cp "#{bindir}/fc-query.exe", TGT_DIR
      cp "#{bindir}/fc-scan.exe", TGT_DIR
      cp "#{bindir}/fc-validate.exe", TGT_DIR
    end
  end

  # common_build is a misnomer. copies prebuilt extentions & gems
  def common_build
    copy_gems
  end
  
end


include FileUtils

class MakeMinGW
  extend Make

  class << self

    def copy_deps_to_dist
      puts "copy_deps_to_dist dir=#{pwd}"
      unless ENV['GDB']
        sh    "#{STRIP}  #{TGT_DIR}/*.dll"
        Dir.glob("#{TGT_DIR}/lib/ruby/**/*.so").each {|lib| sh "#{STRIP} #{lib}"}
      end
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end
 
    # name {TGT_DIR}/shoes
    def make_app(name)
      puts "make_app dir=#{pwd} arg=#{name}"
      bin = "#{name}.exe"
      binc = bin.gsub(/shoes\.exe/, 'cshoes.exe')
      puts "binc  = #{binc}"
      # Detect that msys is being used
      extra = ENV['MSYSTEM_PREFIX'] ? '-DMSYS2' : nil
      rm_f name
      rm_f bin
      rm_f binc
      #extra = ENV['GDB'] == 'profile' ? '-pg' : ''
      sh "#{CC} -o #{bin} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -mwindows -lshoes #{LINUX_LIBS}"
      sh "#{STRIP} #{bin}" unless ENV['GDB']
      sh "#{CC} -o #{binc} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} #{extra} -lshoes #{LINUX_LIBS}"
      sh "#{STRIP} #{binc}" unless ENV['GDB']
   end

    def make_so(name)
      $stderr.puts "make_so dir=#{pwd} arg=#{name}"
      if OBJ.empty?
        $stderr.puts "make_so call not needed"
        return
      end
      #ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
    
    def new_so (name) 
      tgts = name.split('/')
      tgtd = tgts[0]
      $stderr.puts "new_so: #{tgtd}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        #$stderr.puts "collecting .o from #{d}"
        objs = objs + FileList["#{d}/*.o"]      
      end
      # TODO  fix: gtk - needs to dig deeper vs osx
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      #sh "ar -rc #{tgtd}/shoes.lib #{objs.join(' ')}"
      #sh "ranlib #{tgtd}/shoes.lib"    
      sh "#{CC} -o #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def new_link(name)
      tgts = name.split('/')
      tgtd = tgts[0]
      bin = "#{tgtd}/shoes.exe"
      #binc = bin.gsub(/shoes\.exe/, 'cshoes.exe')
      binc = "#{tgtd}/cshoes.exe"
      #puts "binc  = #{binc}"
      #rm_f name
      rm_f bin
      rm_f binc
      sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
      missing = "-lgtk-3 -lgdk-3 -lfontconfig-1 -lpangocairo-1.0" # TODO: This is a bug in env.rb for 
      sh "#{CC} -o #{bin} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes -mwindows  #{LINUX_LIBS} #{missing}"
      sh "#{STRIP} #{bin}" unless ENV['GDB']
      sh "#{CC} -o #{binc} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes #{LINUX_LIBS}  #{missing}"
      sh "#{STRIP} #{binc}" unless ENV['GDB']
      #$stderr.puts "new_link: #{tgtd}"
      #sh "#{CC} -o #{tgts[0]}/shoes  shoes/main.o #{tgtd}/shoes.lib #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
    end   
   
    # does nothing
    def make_userinstall
    end
 
    def make_resource(t)
      puts "make resource"
    end

    Object::APPVERSION = APP['VERSION']
    Object::RELEASE_DATE = Date.parse(APP['DATE']).strftime("%Y-%m-%d")
    def make_installer_gtifw exe_path
      def sh(*args); super; end
      rm_rf "qtifw"
      mkdir_p "qtifw"
      
      cp_r File.join("platform", "msw", "qtifw", "."), "qtifw"
      cp_r File.join(TGT_DIR, "."), File.join("qtifw", "packages", "com.shoesrb.shoes", "data")
      cp "COPYING", File.join("qtifw", "packages", "com.shoesrb.shoes", "meta")
      cp APP['icons']['win32'], File.join("qtifw", "config")

      Dir.chdir("qtifw") do
         rewrite File.join("config", "config-template.xml"), File.join("config", "config.xml")
         rewrite File.join("packages", "com.shoesrb.shoes", "meta", "package-template.xml"), 
           File.join("packages", "com.shoesrb.shoes", "meta", "package.xml")
         #sh "E:/Programmes/Qt/QtIFW/bin/binarycreator -c config/config.xml -p packages #{WINFNAME}.exe"
         sh "#{exe_path} -c config/config.xml -p packages #{WINFNAME}.exe"
      end
      mv "qtifw/#{WINFNAME}.exe", "pkg/qtifw-#{WINFNAME}.exe"
      #rm_rf "qtifw"
     end
   
    def make_installer_nsis exe_path
      def sh(*args); super; end
      puts "make_installer #{`pwd`.chomp} using #{exe_path}"
      mkdir_p "pkg"
      cp_r "VERSION.txt", "#{TGT_DIR}/VERSION.txt"
      rm_rf "#{TGT_DIR}/nsis"
      cp_r  "platform/msw", "#{TGT_DIR}/nsis"
      cp APP['icons']['win32'], "#{TGT_DIR}/nsis/setup.ico"
      rewrite "#{TGT_DIR}/nsis/base.nsi", "#{TGT_DIR}/nsis/#{WINFNAME}.nsi"
      Dir.chdir("#{TGT_DIR}/nsis") do
        #sh "\"c:\\Program Files (x86)\\NSIS\\Unicode\\makensis.exe\" #{WINFNAME}.nsi" 
        sh "#{exe_path} #{WINFNAME}.nsi" 
      end
      mv "#{TGT_DIR}/nsis/#{WINFNAME}.exe", "pkg/"
    end
    
    #Allow diffrent installers
    def make_installer
      if APP['INSTALLER'] == 'qtifw'
        installer = "C:/Qt/QtIFW2.0.5/bin/binarycreator"
        installer = APP['INSTALLER_LOC'] if APP['INSTALLER_LOC']
        make_installer_gtifw installer
      elsif APP['INSTALLER'] == 'nsis'
        installer = "\"c:\\Program Files (x86)\\NSIS\\Unicode\\makensis.exe\""
        installer = APP['INSTALLER_LOC'] if APP['INSTALLER_LOC'] 
        make_installer_nsis installer
      else 
        puts "No Installer defined"
      end
    end
  end
end
