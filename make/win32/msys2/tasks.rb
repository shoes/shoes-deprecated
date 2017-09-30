module Make
  include FileUtils

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
end


include FileUtils

class MakeMinGW
  extend Make

  class << self
    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end

    def make_so(name)
      $stderr.puts "make_so dir=#{pwd} arg=#{name}"
      if OBJ.empty?
        $stderr.puts "make_so call not needed" #TODO: bug in Rakefile
        return
      end
      #ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
    
    def new_so (name) 
      $stderr.puts "new so: #{name}"
      tgtd = File.dirname(name)
      #tgts = name.split('/')
      #tgtd = tgts[0]
      #$stderr.puts "new_so: #{tgtd}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "#{CC} -o #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def new_link(name)
      dpath = File.dirname(name)
      fname = File.basename(name)
      tgts = name.split('/')
      tgtd = tgts[0]
      bin = "#{dpath}/shoes.exe"
      binc = "#{dpath}/cshoes.exe"
      rm_f bin
      rm_f binc
      sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
      tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
      missing = "-lgtk-3 -lgdk-3 -lfontconfig-1 -lpangocairo-1.0" # TODO: a bug in env.rb? 
      sh "#{CC} -o #{bin} #{tp}/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes -mwindows  #{LINUX_LIBS} #{missing}"
      sh "#{STRIP} #{bin}" unless APP['GDB']
      sh "#{CC} -o #{binc} #{tp}/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes #{LINUX_LIBS}  #{missing}"
      sh "#{STRIP} #{binc}" unless APP['GDB']
    end   
   
    # does nothing
    def make_userinstall
    end
 
    def make_resource(t)
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
