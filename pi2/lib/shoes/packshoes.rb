module PackShoes
 require 'shoes/shy'
 require 'shoes/winject'
 require 'rubygems/package'
 require 'zlib'
 require 'fileutils'
# ----- create a shy ------
  def PackShoes.make_shy(dest_path, in_dir, desc)
    #desc is a hash 'name', 'version', 'creator'. 'launch'
    FileUtils.mkdir_p (File.dirname dest_path)
    shy_desc = Shy.new
    for name in desc.keys
      shy_desc.send("#{name}=".intern, desc[name])
    end
    Shy.c(dest_path, shy_desc, in_dir)
  end

# ----- Windows -----

  def PackShoes.dnlif_exe opts
    script = custom_installer opts
    #puts "win script = #{script}"
    size = File.size(script)
    f = File.open(script, 'rb')
    inf = File.join(DIR, "static", "stubs", 'shoes-stub.exe')
    exe = Winject::EXE.new(inf)
    exe.inject_string(Winject::EXE::SHOES_APP_NAME, File.basename(script))
    exe.inject_file(Winject::EXE::SHOES_APP_CONTENT, f.read)
    exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_SITE, opts['dnlhost'])
    exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_PATH, opts['dnlpath'])
    if opts['winargs']
      puts "injecting #{opts['winargs']}"
      exe.inject_string(Winject::EXE::SHOES_USE_ARGS, opts['winargs'])
    end
    if opts['ico']
      exe.inject_icons(opts['ico'])
    end
    exe.save(script.gsub(/\.\w+$/, '') + ".exe") 
    f.close
  end

  def PackShoes.repack_exe (opts, &blk)
    script = custom_installer opts
    size = File.size(script)
    f = File.open(script, 'rb')
    exe = Winject::EXE.new(File.join(DIR, "static", "stubs", "shoes-stub.exe"))
    exe.inject_string(Winject::EXE::SHOES_APP_NAME, File.basename(script))
    exe.inject_file(Winject::EXE::SHOES_APP_CONTENT, f.read)
    exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_SITE, opts['dnlhost'])
    exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_PATH, opts['dnlpath'])
    if opts['winargs']
      puts "injecting #{opts['winargs']}"
      exe.inject_string(Winject::EXE::SHOES_USE_ARGS, opts['winargs'])
    end
    f2 = File.open(opts['shoesdist'],'rb')
    if blk 
      blk.call "Repack Shoes.exe #{opts['shoesdist']} distribution"
    end
    exe.inject_file(Winject::EXE::SHOES_SYS_SETUP, f2.read)
    exe.save(script.gsub(/\.\w+$/, '') + ".exe") do |len|
    end
    f.close
    f2.close
    if blk
      blk.call "Done packaging Windows"
    end
  end
  
# ------ Linux ------

  def PackShoes.dnlif_linux opts
    arch = opts['arch']
    script = custom_installer opts
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tar_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tar"
    tmp_dir = File.join(opts['packtmp'], "+run")
    FileUtils.mkdir_p(tmp_dir)
    # rewrite the static/stubs/sh-install.tmpl to point to the download
    # website, path and script/shy
    File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "sh-install.tmpl"),
	    'HOST' => opts['dnlhost'], 'ARCH' => arch, 
	    'PATH' => opts['dnlpath'],
	    'RELNAME' => opts['relname'],
	    'SCRIPT' => "#{File.basename(script)}"
    end
    FileUtils.cp(script, File.join(tmp_dir, File.basename(script)))
    # add sh-install and script to the modes list
    @tarmodes = {}
    @tarmodes[File.basename(script)] = "0755".oct
    @tarmodes['sh-install'] = "0755".oct
    raw = Shy.du(tmp_dir)
 
    # create a tgz of tmp_dir
    # Create tar file with correct modes (he hopes)
    File.open(tar_path,'wb') do |tf|
      tb = fastcf(tf, tmp_dir, @tarmodes)
    end
    # compress tar file 
    File.open(tgz_path,'wb') do |tgz|
      z = Zlib::GzipWriter.new(tgz)
      File.open(tar_path,'rb') do |tb|
        z.write tb.read
      end
      z.close
    end
    FileUtils.rm tar_path
    
    md5, fsize = Shy.md5sum(tgz_path), File.size(tgz_path)
    File.open(run_path, 'wb') do |f|
	  rewrite f, File.join(DIR, "static", "stubs", "blank.run"),
	   'CRC' => '0000000000', 'MD5' => md5, 'LABEL' => app_name, 'NAME' => name,
	   'SIZE' => fsize, 'RAWSIZE' => (raw / 1024) + 1, 'TIME' => Time.now, 'FULLSIZE' => raw
      File.open(tgz_path, 'rb') do |f2|
	     f.write f2.read(8192) until f2.eof
	  end
    end
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
   
  end

  def PackShoes.repack_linux opts, &blk
    #opts.each {|k, v| puts "#{k} => #{v}"}
    arch = opts['arch']
    script = custom_installer opts
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tar_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tar"
    tmp_dir = File.join(opts['packtmp'], "+run")
    FileUtils.mkdir_p(tmp_dir)
    pkgf = open(opts['shoesdist'],'rb')  # downloaded/cached .run/.install
    if blk 
      blk.call"Expanding #{arch} distribution. Patience is needed"
    end
    # skip to the tar contents in the .run
	size = Shy.hrun(pkgf)
	# Copy the rest to a new file
	wk_dir = File.join(opts['packtmp'], "+tmp")
    FileUtils.mkdir_p(wk_dir)
	wkf = open(File.join(wk_dir,"run.gz"),'wb')
	buff = ''
	while pkgf.read(32768, buff) != nil do
	   wkf.write(buff) 
    end
	wkf.close
	if blk 
	  blk.call "Start extract"
	end
	@tarmodes = {}
	fastxzf(wkf, tmp_dir, @tarmodes)
    FileUtils.rm_rf(wk_dir)
    if blk 
      blk.call "Copy script and stubs"
    end
    FileUtils.cp(script, File.join(tmp_dir, File.basename(script)))
    File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "sh-install"),
	    'SCRIPT' => "./#{File.basename(script)}"
    end
    FileUtils.chmod 0755, File.join(tmp_dir, "sh-install")
    # add sh-install and script to the modes list
    @tarmodes[File.basename(script)] = "0755".oct
    @tarmodes['sh-install'] = "0755".oct
    # debug - dump @tarmodes
    #@tarmodes.each { |k,v| puts "#{k} #{sprintf('%4o',v)}" }
    
 
    if blk
      blk.call "Compute size and compress"
    end
    raw = Shy.du(tmp_dir)
        
    #Create tar file with correct modes (he hopes)
    File.open(tar_path,'wb') do |tf|
      tb = fastcf(tf, tmp_dir, @tarmodes)
    end
    # compress tar file 
    File.open(tgz_path,'wb') do |tgz|
      z = Zlib::GzipWriter.new(tgz)
      File.open(tar_path,'rb') do |tb|
        z.write tb.read
      end
      z.close
    end
    FileUtils.rm tar_path
    if blk 
      blk.call "Checksum, install  and copy"
    end
    md5, fsize = Shy.md5sum(tgz_path), File.size(tgz_path)
    File.open(run_path, 'wb') do |f|
	  rewrite f, File.join(DIR, "static", "stubs", "blank.run"),
	   'CRC' => '0000000000', 'MD5' => md5, 'LABEL' => app_name, 'NAME' => name,
	   'SIZE' => fsize, 'RAWSIZE' => (raw / 1024) + 1, 'TIME' => Time.now, 'FULLSIZE' => raw
      File.open(tgz_path, 'rb') do |f2|
	     f.write f2.read(8192) until f2.eof
	  end
    end
    if blk
      blk.call "Done packing Linux"
    end
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
  end
  
# ------ OSX -----

  def PackShoes.dnlif_osx opts
    script = custom_installer opts
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    #vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
    tar_path = script.gsub(/\.\w+$/, '') + "-osx.tar"
    tgz_path = script.gsub(/\.\w+$/, '') + "-osx.tgz"
    app_app = "#{app_name}.app"
    vers = [2, 0]

    tmp_dir = File.join(opts['packtmp'], "+dmg")
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # deal with custom icons. Brutish
    icon_path = ''
    if opts['advopts'] && opts['icns'] 
      icon_path = opts['icns']
    else
      # use Shoes
      icon_path = File.join(DIR, "static", "Shoes.icns")
    end
    icon_name = File.basename(icon_path)
     
    @tarmodes = {}
 
    app_dir = File.join(tmp_dir, app_app)

    res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
    mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
    [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
    #FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
    #FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
    FileUtils.cp(icon_path, app_dir)
    FileUtils.cp(icon_path, res_dir)
    # make cache entries for the files above to keep the console quiet
    #@tarmodes["#{app_app}/Contents/Resources/Shoes.icns"] = 0644
    #@tarmodes["#{app_app}/Shoes.icns"] = 0644
    @tarmodes["#{app_app}/Contents/Resources/#{icon_name}"] = 0644
    @tarmodes["#{app_app}/#{icon_name}"] = 0644
    # more permissions cache entries 
    ["#{app_app}", "#{app_app}/Contents", "#{app_app}/Contents/Resources",
     "#{app_app}/Contents/MacOS" ].each {|d| @tarmodes[d] = 0755}
    ["#{app_app}/Contents/PkgInfo", "#{app_app}/Contents/version.plist",
     "#{app_app}/Contents/Info.plist" ].each {|f| @tarmodes[f] = 0644}
    File.open(File.join(app_dir, "Contents", "PkgInfo"), 'w') do |f|
      f << "APPL????"
    end
    # info.plist with substitutions
    #@dnlsel = '/public/select/osx.rb'
    File.open(File.join(app_dir, "Contents", "Info.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleGetInfoString</key>
  <string>#{app_name} #{vers.join(".")}</string>
  <key>CFBundleExecutable</key>
  <string>osx-app</string>
  <key>CFBundleIdentifier</key>
  <string>org.hackety.#{name}</string>
  <key>CFBundleName</key>
  <string>#{app_name}</string>
  <key>CFBundleIconFile</key>
  <string>#{icon_name}</string>
  <key>CFBundleShortVersionString</key>
  <string>#{vers.join(".")}</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>IFMajorVersion</key>
  <integer>#{vers[0]}</integer>
  <key>IFMinorVersion</key>
  <integer>#{vers[1]}</integer>
</dict>
</plist>
END
    end
    File.open(File.join(app_dir, "Contents", "version.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>BuildVersion</key>
  <string>1</string>
  <key>CFBundleVersion</key>
  <string>#{vers.join(".")}</string>
  <key>ProjectName</key>
  <string>#{app_name}</string>
  <key>SourceVersion</key>
  <string>#{Time.now.strftime("%Y%m%d")}</string>
</dict>
</plist>
END
    end
    File.open(File.join(mac_dir, "#{name}-launch"), 'wb') do |f|
      f << <<END
#!/bin/bash
APPPATH="${0%/*}"
this_dir=$APPPATH
unset DYLD_LIBRARY_PATH
APPPATH=/Applications/Shoes.app/Contents/MacOS
cd "$APPPATH"
echo "[Pango]" > pangorc
echo "ModuleFiles=$APPPATH/pango.modules" >> pangorc
echo "ModulesPath=$APPPATH/pango/modules" >> pangorc
PANGO_RC_FILE="$APPPATH/pangorc" ./pango-querymodules > pango.modules
DYLD_LIBRARY_PATH="$APPPATH" PANGO_RC_FILE="$APPPATH/pangorc" SHOES_RUBY_ARCH="#{opts['shoesruby']}" ./shoes-bin "$this_dir/#{File.basename(script)}"
END
    end
    ls = File.join(mac_dir, "#{name}-launch")
    FileUtils.chmod 0755, ls
    @tarmodes["#{app_app}/Contents/MacOS/#{name}-launch"] = 0755
    FileUtils.cp(script, File.join(mac_dir, File.basename(script)))
    @tarmodes["#{app_app}/Contents/MacOS/#{File.basename(script)}"] = 0644
    # copy the downloader 
    FileUtils.cp(File.join(DIR, "static", "stubs", "shoes-osx-install"),
      File.join(mac_dir, "shoes-osx-install"))
    @tarmodes["#{app_app}/Contents/MacOS/shoes-osx-install"] = 0755
    
    # make the 1st script to run (installs shoes if needed, runs Shoes
    File.open(File.join("#{mac_dir}", "osx-app"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "osx-app-install.tmpl"),
	    'HOST' => "http://#{opts['dnlhost']}", 'ARCH' => opts['arch'], 
	    'PATH' => opts['dnlpath'],
	    'RELNAME' => opts['relname'],
	    'SCRIPT' => "#{name}-launch"
    end
    @tarmodes["#{app_app}/Contents/MacOS/osx-app"] = 0755
    
    #Create tar file with correct modes (he hopes)
    File.open(tar_path,'wb') do |tf|
      tb = fastcf(tf, tmp_dir, @tarmodes)
    end
    # compress tar file 
    File.open(tgz_path,'wb') do |tgz|
      z = Zlib::GzipWriter.new(tgz)
      File.open(tar_path,'rb') do |tb|
        z.write tb.read
      end
      z.close
    end
#    FileUtils.rm_rf(tmp_dir)
    FileUtils.rm_rf(tar_path)
    #@pkgstat = inscription "Done packaging #{$script_path} for OSX"
 end

  
  def PackShoes.repack_osx opts, &blk
    
    script = custom_installer opts
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    #vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
    tar_path = script.gsub(/\.\w+$/, '') + "-osx.tar"
    tgz_path = script.gsub(/\.\w+$/, '') + "-osx.tgz"
    app_app = "#{app_name}.app"
    vers = [1, 0]

    tmp_dir = File.join(opts['packtmp'], "+dmg")
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # expand download into ~/.shoes/+dmg/Shoes.app
    pkgf = open(opts['shoesdist'], 'rb')
    if blk
      blk.call"Expanding OSX distribution. Patience is needed"
    end
	#Shy.xzf(pkgf, tmp_dir)   
	@tarmodes = {}
	fastxzf(pkgf, tmp_dir, @tarmodes, app_app)
	# debug 
	# @tarmodes.each_key {|k| puts "entry #{k} = #{@tarmodes[k]}" }
    app_dir = File.join(tmp_dir, app_app)
    # rename Shoes.app to app_dir
    Dir.chdir tmp_dir do
      FileUtils.mv "Shoes.app", app_app
    end
    # deal with custom icons. Brutish
    icon_path = ''
    if  opts['icns'] 
      icon_path = opts['icns']
    else
      # use Shoes icons
      icon_path = File.join(DIR, "static", "Shoes.icns")
    end
    icon_name = File.basename(icon_path)
 
    res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
    mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
    [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
    #FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
    #FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
    FileUtils.cp(icon_path, app_dir)
    FileUtils.cp(icon_path, res_dir)
    # make cache entries for two files above just to keep the consoles
    # messages away. 
    @tarmodes["#{app_app}/Contents/Resources/#{icon_name}"] = 0644
    @tarmodes["#{app_app}/#{icon_name}"] = 0644
    File.open(File.join(app_dir, "Contents", "PkgInfo"), 'w') do |f|
      f << "APPL????"
    end
    File.open(File.join(app_dir, "Contents", "Info.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleGetInfoString</key>
  <string>#{app_name} #{vers.join(".")}</string>
  <key>CFBundleExecutable</key>
  <string>#{name}-launch</string>
  <key>CFBundleIdentifier</key>
  <string>org.hackety.#{name}</string>
  <key>CFBundleName</key>
  <string>#{app_name}</string>
  <key>CFBundleIconFile</key>
  <string>#{icon_name}</string>
  <key>CFBundleShortVersionString</key>
  <string>#{vers.join(".")}</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>IFMajorVersion</key>
  <integer>#{vers[0]}</integer>
  <key>IFMinorVersion</key>
  <integer>#{vers[1]}</integer>
</dict>
</plist>
END
    end
    File.open(File.join(app_dir, "Contents", "version.plist"), 'w') do |f|
      f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>BuildVersion</key>
  <string>1</string>
  <key>CFBundleVersion</key>
  <string>#{vers.join(".")}</string>
  <key>ProjectName</key>
  <string>#{app_name}</string>
  <key>SourceVersion</key>
  <string>#{Time.now.strftime("%Y%m%d")}</string>
</dict>
</plist>
END
    end
    File.open(File.join(mac_dir, "#{name}-launch"), 'wb') do |f|
      f << <<END
#!/bin/bash
APPPATH="${0%/*}"
unset DYLD_LIBRARY_PATH
cd "$APPPATH"
echo "[Pango]" > pangorc
echo "ModuleFiles=$APPPATH/pango.modules" >> pangorc
echo "ModulesPath=$APPPATH/pango/modules" >> pangorc
PANGO_RC_FILE="$APPPATH/pangorc" ./pango-querymodules > pango.modules
DYLD_LIBRARY_PATH="$APPPATH" PANGO_RC_FILE="$APPPATH/pangorc" SHOES_RUBY_ARCH="#{opts['shoesruby']}" ./shoes-bin "#{File.basename(script)}"
END
    end
    ls = File.join(mac_dir, "#{name}-launch")
    FileUtils.chmod 0755, ls
    @tarmodes["#{app_app}/Contents/MacOS/#{name}-launch"] = 0755
    FileUtils.cp(script, File.join(mac_dir, File.basename(script)))
    @tarmodes["#{app_app}/Contents/MacOS/#{File.basename(script)}"] = 0644
    #FileUtils.cp(File.join(DIR, "static", "stubs", "cocoa-install"),
    #  File.join(mac_dir, "cocoa-install"))
    if blk
      blk.call "Creating new archive"
    end
    #File.open(tgz_path, 'wb') do |f|
	#  Shy.czf(f, tmp_dir)
    #end
    # #Create tar file with correct modes (he hopes)
    File.open(tar_path,'wb') do |tf|
      tb = fastcf(tf, tmp_dir, @tarmodes)
    end
    # compress tar file 
    File.open(tgz_path,'wb') do |tgz|
      z = Zlib::GzipWriter.new(tgz)
      File.open(tar_path,'rb') do |tb|
        z.write tb.read
      end
      z.close
    end
    FileUtils.rm_rf(tmp_dir)
    FileUtils.rm_rf(tar_path)
  end

# ------- Helpers ------
  # only sub matches. 
  def PackShoes.rewrite a, before, hsh
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(/\#\{(\w+)\}/) {
          if hsh[$1] 
            hsh[$1]
          else
            '#{'+$1+'}'
          end
        }
      end
    end
  end

  def PackShoes.fastxzf infile, outdir, modes = {}, osx = ''
	#from blog post http://dracoater.blogspot.com/2013/10/extracting-files-from-targz-with-ruby.html
	# modified by Cecil Coupe - Jun 27+, 2014. Thanks to Juri Timošin   
	
	tar_longlink = '././@LongLink'

	Gem::Package::TarReader.new( Zlib::GzipReader.open infile ) do |tar|
	  dest = nil
	  tar.each do |entry|
	    if entry.full_name == tar_longlink
	      dest = File.join outdir, entry.read.strip
	      next
	    end
	    dest ||= File.join outdir, entry.full_name
	    hashname = entry.full_name.gsub('./','')
	    hashname.gsub!(/Shoes.app/, osx) if osx
	    hashname.chomp!('/')
	    #@pkgstat.text = hashname
	    modes[hashname] = entry.header.mode
	    if entry.directory?
	      FileUtils.rm_rf dest unless File.directory? dest
	      FileUtils.mkdir_p dest, :mode => entry.header.mode, :verbose => false
	    elsif entry.file?
	      FileUtils.rm_rf dest unless File.file? dest
	      File.open dest, "wb" do |f|
	        f.print entry.read
	      end
	      FileUtils.chmod entry.header.mode, dest, :verbose => false
	    elsif entry.header.typeflag == '2' #Symlink!
	      #puts "Symlink #{entry.header.linkname} Ignore"
	      # File.symlink entry.header.linkname, dest
	    end
	    dest = nil
	  end
	end
  end
  
  def PackShoes.fastcf outf, indir, modes
    begin
	    Gem::Package::TarWriter.new outf do |tar|
	      Dir[File.join(indir, "**/*")].each do |file|
	        relative_file = file.sub(/^#{Regexp::escape indir}\/?/, '')
	        mode = modes[relative_file] 
	        if !mode
	          puts "Failed #{relative_file} #{file}"
	          mode = File.stat(file).mode
	        end
	        if File.directory?(file)
	          tar.mkdir relative_file, mode
	        else
	          tar.add_file relative_file, mode do |tf|
	            File.open(file, "rb") { |f| tf.write f.read }
	          end
	        end
	      end #Dir
	    end # Tarwriter	  
	rescue StandardError => e
	  puts "Exception: #{e}"
	end
	outf.rewind
	outf
  end

  def PackShoes.custom_installer opts
    defshy = opts['app']
    if opts['advopts'] != true
        return defshy
    end
    #opts.each {|k, v| puts "#{k} => #{v}"}
    if ! defshy[/\.shy$/] then
        raise "Must be a shy"
    end
    appname = File.basename(defshy,".shy")
    shydir = File.dirname(defshy)
    tmp_dir = File.join(opts['packtmp'], "+shy", appname)
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # copy shy to tmp - TODO: Copy User Script here
    FileUtils.cp(defshy, tmp_dir)
    # copy app-install.tmpl with rewrite
    File.open(File.join(tmp_dir, "#{appname}-install.rb"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "app-install.tmpl"),
	    'SHYFILE' => "#{defshy}"
    end
    # TODO: Copy gems.zip here.
    FileUtils.cp(opts['png'], File.join(tmp_dir,"#{appname}.png")) if opts['png']
    FileUtils.cp(opts['ico'], File.join(tmp_dir,"#{appname}.ico")) if opts['ico']
    FileUtils.cp(opts['icns'], File.join(tmp_dir,"#{appname}.icns")) if opts['icns']
    # make a yaml to pass options the installer might need (expandshy for one)
    File.open(File.join(tmp_dir,"#{appname}.yaml"),'w') {|f| YAML.dump(opts, f)}
    # Create a shy header for the installer.
    shy_desc = Shy.new
    shy_desc.launch = "#{appname}-install.rb"
    shy_desc.name = appname
    shy_desc.version = "1.0"
    shy_desc.creator = "Shoes"
    # Create the new shy. Grrr -- read shy.rb
    new_shypath = File.join(opts['packtmp'], '+shy')
    Dir.chdir(new_shypath) do
      Shy.c(appname+'.shy', shy_desc, tmp_dir)
    end
    #puts "New: #{new_shypath}\n and #{defshy}"
    # write over the incoming shy with the new one - Bad Idea. Bad Cecil!
    #FileUtils.cp File.join(new_shypath,"#{appname}.shy"), defshy
    FileUtils.cp File.join(new_shypath,"#{appname}.shy"), File.join(shydir,"#{appname}-custom.shy")
    # delete the +shy temp stuff
    #return defshy
    return File.join(shydir,"#{appname}-custom.shy")
  end

end
