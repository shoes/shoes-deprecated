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

    def copy_deps_to_dist
      puts "copy_deps_to_dist dir=#{pwd}"
      unless APP['GDB']
        sh "#{STRIP}  #{TGT_DIR}/*.dll"
        Dir.glob("#{TGT_DIR}/lib/ruby/**/*.so").each {|lib| sh "#{STRIP} #{lib}"}
      end
    end

    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end

    def make_so(name)
      puts "make_so dir=#{pwd} arg=#{name}"
      if OBJ.empty?
        $stderr.puts "make_so called in error"
        return
      end
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
    
    # this is called from the file task based new_builder 
    def new_so (name) 
      tgts = name.split('/')
      tgtd = tgts[0]
      $stderr.puts "new_so: #{tgtd}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      # TODO  fix: gtk - needs to dig deeper vs osx
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "#{CC} -o #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end

    def new_link(name)
      tgts = name.split('/')
      tgtd = tgts[0]
      bin = "#{tgtd}/shoes.exe"
      binc = "#{tgtd}/cshoes.exe"
      rm_f bin
      rm_f binc
      sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
      missing = "-lgtk-3 -lgdk-3 -lfontconfig-1 -lpangocairo-1.0" # TODO: This is a bug in env.rb for 
      sh "#{CC} -o #{bin} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes -mwindows  #{LINUX_LIBS} #{missing}"
      sh "#{STRIP} #{bin}" unless APP['GDB']
      sh "#{CC} -o #{binc} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes #{LINUX_LIBS}  #{missing}"
      sh "#{STRIP} #{binc}" unless APP['GDB']
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
      end
      mv "#{TGT_DIR}/nsis/#{WINFNAME}.exe", "pkg/"
    end

  end
end
