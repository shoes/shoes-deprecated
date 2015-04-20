# build Shoes gems to be copied later. Fluff is not copied - Just the
# gemspec, LICENSE lib/ and the right arch so/dll/dylib

module Make
  include FileUtils
  Lext = DLEXT == 'dylib'? 'bundle' : 'so'  
  
  # note: Some methods are unique to invoking context and
  # may not have the Constants set for other contexts
  
    def build_all_gems 
      gems = Dir.glob("#{APP['GEMLOC']}/*")
      APP['EXTLIST'].each {|p| gems.delete("#{APP['EXTLOC']}/#{p}") }
      gems.each do |gemp|
        next if gemp =~ /built$/
        build_gem gemp
        clean_gem gemp # clean out binary stuff
      end
    end
    
    def build_shoes_gems
      APP['EXTLIST'].each do |nm| 
        build_shoes_ext "#{APP['EXTLOC']}/#{nm}"
        clean_gem "#{APP['EXTLOC']}/#{nm}"
      end
      APP['GEMLIST'].each do |nm|
        build_gem "#{APP['GEMLOC']}/#{nm}"
        clean_gem "#{APP['GEMLOC']}/#{nm}"
      end
    end

    def build_shoes_ext xdir
      gemn = File.basename(xdir)
      puts "Building #{gemn} in #{xdir}"
      dest = "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{gemn}"
      rm_rf dest
      mkdir_p dest
      Dir.glob("#{xdir}/ext/*").each do |ext|
        Dir.chdir(ext) do
        extcnf = (File.exists? "#{TGT_ARCH}-extconf.rb") ? "#{TGT_ARCH}-extconf.rb" : 'extconf.rb'
        unless system "ruby", "#{extcnf}" and system "make"
          raise "Gem build failed"
        end
        Dir.glob("*.#{Lext}").each do |so|
          mkdir_p "#{dest}/ext"
          cp so, "#{dest}/ext" 
        end
       end
      end
      cp_r "#{xdir}/lib/", dest if File.exists? "#{xdir}/lib"  

    end
    
    def build_gem xdir 
      gemn = File.basename(xdir)
      puts "Building #{gemn} in #{xdir}"
      Dir.glob("#{xdir}/ext/*").each do |ext|
        Dir.chdir(ext) do
        extcnf = (File.exists? "#{TGT_ARCH}-extconf.rb") ? "#{TGT_ARCH}-extconf.rb" : 'extconf.rb'
        unless system "ruby", "#{extcnf}" and system "make"
          raise "Gem build failed"
        end
       end
      end
      # copy to GEMLOC/built/TGT_ARCH/gemname for later inclusion 
      # into shoes build, package, gempack
      dest = "#{APP['GEMLOC']}/built/#{TGT_ARCH}/#{gemn}"
      rm_rf dest
      mkdir_p dest
      cp "#{xdir}/gemspec", dest
      cp "#{xdir}/LICENSE", dest
      cp_r "#{xdir}/lib/", dest
      Dir.glob("#{xdir}/ext/*/*.#{Lext}").each {|so| cp so, "#{dest}/lib" }
    end
    
    def clean_all_gems
      Dir.glob("#{APP['GEMLOC']}/*").each do |gemp|
        next if gemp =~ /built$/
        puts "clean #{gemp}"
        clean_gem gemp
      end
      rm_rf "#{APP['GEMLOC']}/built/#{TGT_ARCH}"
    end
    
    def clean_gem gemp
      gemn = File.basename(gemp)
      Dir.glob("#{gemp}/ext/*").each do |ext|
        Dir.chdir(ext) do |f|
          Dir.glob('*.o').each {|f| rm_r f}
          Dir.glob('*.so').each {|f| rm_r f}
          Dir.glob('*.bundle').each {|f| rm_r f}
          Dir.glob('*.def').each {|f| rm_r f}
        end
      end
    end
    
      def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end

  #  ---- this method is called from the shoes build
  def copy_gems
    puts "copy_gems dir=#{pwd} #{SHOES_TGT_ARCH}"
    APP['EXTLIST'].each do |ext|
      puts "copy prebuild ext #{ext}"
      copy_files "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/ext/*.#{Lext}", "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_TGT_ARCH}" 
      if  File.exists? "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/lib"
        Dir.glob("#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/lib/*").each do |lib|
          cp_r lib, "#{TGT_DIR}/lib/ruby/#{RUBY_V}"
        end
      end
    end
    gdir = "#{TGT_DIR}/lib/ruby/gems/#{RUBY_V}"
    # precompiled gems here - just copy
    APP['GEMLIST'].each do |gemn|
      gemp = "#{APP['GEMLOC']}/built/#{TGT_ARCH}/#{gemn}" 
      puts "Copying prebuilt gem #{gemp}"
      spec = eval(File.read("#{gemp}/gemspec"))
      mkdir_p "#{gdir}/specifications"
      mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
      FileList["#{gemp}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
      cp "#{gemp}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
    end
  end

    
end

desc "build all gems for platform"
task :gems  do
  Builder.build_all_gems
end

desc "Build internal Shoes gems and ext"
task :shoesgems do
  Builder.build_shoes_gems
end

desc "clean all gem for platform"
task :gemsclean do
  Builder.clean_all_gems
end

if !APP['GEMLOC']
  abort "You must define APP['GEMLOC'] see make/#{TGT_ARCH}/env.rb"
end
