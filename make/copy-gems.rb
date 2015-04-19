module Make
  include FileUtils
  
 
  def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end

  
  def copy_gems
    puts "copy_gems dir=#{pwd} #{SHOES_TGT_ARCH}"
    gext = DLEXT == 'dylib'? 'bundle' : 'so'
    APP['EXTLIST'].each do |ext|
      puts "copy prebuild ext #{ext}"
      copy_files "#{APP['EXTLOC']}/built/#{TGT_ARCH}/#{ext}/ext/*.#{gext}", "#{TGT_DIR}/lib/ruby/#{RUBY_V}/#{SHOES_TGT_ARCH}" 
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
