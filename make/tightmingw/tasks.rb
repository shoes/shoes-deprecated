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
    puts "pre_build dir=#{`pwd`}"
    rbvt = RUBY_V
    rbvm = RUBY_V[/^\d+\.\d+/]
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
      cp "#{ShoesDeps}/share/glib-2.0/schemas/gschema.dtd",
        "#{TGT_DIR}/share/glib-2.0/schemas"
    else
      cp  "#{TGT_SYS_DIR}/share/glib-2.0/schemas/gschemas.compiled" ,
        "#{TGT_DIR}/share/glib-2.0/schemas"
    end
    sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
    cp 'platform/msw/fonts.conf', TGT_DIR
 end

  # common_build is a misnomer. Builds extentions, gems
  def common_build
    puts "common_build dir=#{pwd} #{SHOES_TGT_ARCH}"
    %w[req/ftsearch/lib/* req/rake/lib/*].each do |rdir|
      FileList[rdir].each { |rlib| cp_r rlib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}" }
    end
    #%w[req/binject/ext/binject_c req/ftsearch/ext/ftsearchrt req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
    %w[req/ftsearch/ext/ftsearchrt req/chipmunk/ext/chipmunk].
      each { |xdir| copy_ext xdir, "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_TGT_ARCH}" }

    gdir = "#{TGT_DIR}/lib/ruby/gems/#{RUBY_V}"
    #{'hpricot' => 'lib', 'json' => 'lib/json/ext', 'sqlite3' => 'lib'}.each do |gemn, xdir|
    {'hpricot' => 'lib', 'sqlite3' => 'lib'}.each do |gemn, xdir|
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

class MakeMinGW
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
      rm_f name
      rm_f bin
      rm_f binc
      extra = ENV['GDB'] == 'profile' ? '-pg' : ''
      sh "#{CC} -o #{bin} bin/main.o shoes/appwin32.o -L#{TGT_DIR} -mwindows -lshoes #{LINUX_LIBS}"
      sh "#{STRIP} #{bin}" unless ENV['GDB']
      sh "#{CC} -o #{binc} bin/main.o shoes/appwin32.o -L#{TGT_DIR} #{extra} -lshoes #{LINUX_LIBS}"
      sh "#{STRIP} #{binc}" unless ENV['GDB']
   end

    def make_so(name)
      puts "make_so dir=#{pwd} arg=#{name}"
      #ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
   
    # does nothing
    def make_userinstall
    end
 
    def make_resource(t)
      puts "make resource"
    end

    
    def make_installer
      # assumes you have NSIS installed on your box in the system PATH 
      def sh(*args); super; end
      puts "make_installer #{`pwd`}"
      mkdir_p "pkg"
      cp_r "VERSION.txt", "#{TGT_DIR}/VERSION.txt"
      rm_rf "#{TGT_DIR}/nsis"
      cp_r  "platform/msw", "#{TGT_DIR}/nsis"
      cp APP['icons']['win32'], "#{TGT_DIR}/nsis/setup.ico"
      rewrite "#{TGT_DIR}/nsis/base.nsi", "#{TGT_DIR}/nsis/#{WINFNAME}.nsi"
      Dir.chdir("#{TGT_DIR}/nsis") do
        sh "\"c:\\Program Files (x86)\\NSIS\\Unicode\\makensis.exe\" #{WINFNAME}.nsi" 
        #sh "c:\\Program Files (x86)\\NSIS\\Unicode\\makensis.exe #{WINFNAME}.nsi"
      end
      mv "#{TGT_DIR}/nsis/#{WINFNAME}.exe", "pkg/"
    end

  end
end
