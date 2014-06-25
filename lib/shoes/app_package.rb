# The 'New' packager for Shoes 3.2
require 'shoes/shy'
require 'binject'
require 'open-uri'

Shoes.app do
  # get the urls or default
  @shoes_home = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}"
  @selurl = "#{@shoes_home}/package/selector.url"
  if File.exists? @selurl
    File.open(@selurl,"r") do |f|
      @home_site = f.read
    end
  else
    @home_site = 'http://shoes.mvmanila.com/public/select/pkg.rb'
  end
  @dnlurl = "#{@shoes_home}/package/download.url"
  if File.exists? @dnlurl
    File.open(@dnlurl,"r") do |f|
      @dnl_site = f.read.strip
      @dnl_site.gsub!(/\/$/,'')
    end
  else
    @dnl_site = 'http://shoes.mvmanila.com/public/shoes'
  end
  
  stack do
    para "Package A Shoes Application"
    selt = proc { @sel1.toggle; @sel2.toggle }
    @path = ""
    @shy_path = nil
    @sel1 =
      flow do
	    para "File to package:"
	    inscription " (or a ", link("directory", &selt), ")"
	    edit1 = edit_line :width => -120
	    @bb = button "Browse...", :width => 100 do
		  @path = edit1.text = ask_open_file
		  #est_recount
	    end
	  end
    @sel2 =
	  flow :hidden => true do
	    para "Directory:"
	    inscription " (or a ", link("single file", &selt), ")"
	    edit2 = edit_line :width => -120
	    @bf = button "Folder...", :width => 100 do
		  @path = edit2.text = ask_open_folder
		  #est_recount
	    end
	  end

    @menu_panel = stack do
      flow do 
        button 'Download Options' do
          download @home_site do |r| 
            #para "Status: #{r.response.status}\nHeaders:"
            #r.response.headers.each {|k,v| para "#{k}: #{v}\n"}
            sz = r.response.body.size
            if sz > 0
              @platforms = {}
              lines = r.response.body.split("\n")
              lines.each do |ln|
                platform_merge ln.strip
              end
              @select_panel.clear
              @select_panel.background white
			  @platforms.each_key do |k| 
			    ln = @platforms[k]
                flds = ln.split(' ')
                parts = flds[2].split('-')
                @select_panel.append do 
                  flow margin: 5 do
                    para "#{flds[0]} MB  "
                    button "#{k}", width: 200 do
                      platform_download flds[2], flds[0], flds[1]
                    end
                    para " #{parts[0]}  #{parts[1]}"
                  end
                end
			  end
            else
              @info_panel = para "Failed"
            end
          end
        end
        button 'Quit' do
          exit
        end
      end
    end
    @select_panel = stack width: 0.90, left_margin: 10 do
      background white
    end
    # layout the gui elements now
    @info_panel = stack do
    end
  end
  
  # FIXME: Assumes lines are sorted by filename at server
  def platform_merge ln
    flds = ln.split(' ')
    return if flds[0].to_i == 0
    # app-version-arch'
    fname = flds[2]
    #parts = fname.split('-')
    #return if parts[length] < 3
    #puts "fname = #{fname}"
    case ln
      when /32\.exe$/
        @platforms['Win32'] = ln
      when /\.tbz$/
        return  # ignore 
      when /osx\-.*\.tgz$/
        @platforms['OSX'] = ln
      when /armhf\.run$/
        @platforms['Linux_Raspberry'] = ln
      when /i686\.run$/
        @platforms['Linux_i686'] = ln
      when /x86_64\.run$/ 
        @platforms['Linux_x86_64'] = ln
      when /tar\.gz$/
        tarball = ln
      else
        puts "failed match #{ln}"
    end
    return
  end
  
  #here's where the fun begins. Processing one item.  Download it, pull it
  # apart, put the app in and put it back together. Or something like that.
  def platform_download fname, szMB, timestamp
    if @path == '' 
      alert "Please enter a file name or directory"
      return
    end
    dnlsize = szMB.to_i*(1024*1024)
    dnlts = timestamp.to_i
    # setup a place/file to download to
    work_dir = File.join(LIB_DIR, Shoes::RELEASE_NAME.downcase, 'package')
    FileUtils.makedirs work_dir
    @dnlurl = "#{@dnl_site}/#{fname}"
    @work_path = "#{work_dir}/#{fname}"
    @info_panel.clear
    # check if we have already downloaded this one - arguably the confirm
    # is not needed.
    @download_needed = false
    if !File.exists?(@work_path) || (
       File.mtime(@work_path).to_i <= dnlts ||
       File.size(@work_path).to_i <= dnlsize  ||
       !(confirm "Use cached #{@work_path} ?\nCancel will re-download"))
    then
      @download_needed = true
    end
    if @download_needed 
      @info_panel.append do 
        background "#eee".."#ccd"
        @dnlpanel = stack :margin => 10 do
          dld = nil
          @dnlmenu= para @dnlurl, " [", link("cancel") { @dlnthr.exit }, "]", :margin => 0
          @dnlstat = inscription "Beginning transfer.", :margin => 0
          @dnlbar = progress :width => 1.0, :height => 14 
	      @dlnthr = download @dnlurl, :save =>  @work_path,
		     :progress => proc { |dl| 
		          @dnlstat.text = "Transferred #{dl.transferred} of #{dl.length} bytes (#{sprintf('%2i',dl.percent * 100)}%)"
		          @dnlbar.fraction = dl.percent 
		         },
		      :finish => proc { |dl| 
		          @dnlstat.text = "Download completed"
		          platform_repack
		        }
	    end
      end
    else
      platform_repack
    end
  end
  
  # We've got @vars with all kinds of info. 
  def platform_repack 
    @info_panel_clear
    @info_panel.append do
      case @work_path
      when /\.run$/
        Thread.new do
          @pkgstat = inscription "Linux repack #{@path} for#{@work_path}"
          @pkgbar = progress :width => 1.0, :height => 14 
          arch = @work_path[/(\w+)\.run$/] 
          arch.gsub!('.run','')
          repack_linux arch
        end
      when /.exe$/
        Thread.new do
          @pkgstat = inscription "Windows repack #{@path} for#{@work_path}"
          repack_exe
        end
      when /osx\-.*.tgz$/
        Thread.new do
          @pkgstat = inscription "OSX repack #{@path} for#{@work_path}"
          repack_osx
        end
      else
        para "don't know what to do with #{@work_path}"
      end
    end
  end
  
  def rewrite a, before, hsh
    File.open(before) do |b|
      b.each do |line|
        a << line.gsub(/\#\{(\w+)\}/) { hsh[$1] }
      end
    end
  end
  
  def repack_linux  arch
    script = @path
    sofar, stage = 0.0, 3.0 
    blk = proc do |frac|
      #@pkgbar.style(:width => sofar + (frac * stage))
      @pkgbar.fraction = sofar + (frac * stage)
      #puts "progress #{frac}"
    end
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tmp_dir = File.join(LIB_DIR, "+run")
    FileUtils.mkdir_p(tmp_dir)
    # pkgf = pkg("linux", opt)
    pkgf = open(@work_path)
    prog = 1.0
    @pkgstat.text = "Expanding #{arch} distribution. Patience is needed"
    if pkgf
	  size = Shy.hrun(pkgf)
	  pblk = Shy.progress(size) do |name, perc, left|
	    # @pkgdetails.text = name
	    blk[perc * 0.5]
	  end if blk
	  #Shy.xzf(pkgf, tmp_dir, &pblk)
	  Shy.xzf(pkgf, tmp_dir)
	  sofar = 0.33
    end
    @pkgstat.text = "Copy script and stubs"
    FileUtils.cp(script, File.join(tmp_dir, File.basename(script)))
    File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "sh-install"),
	    'SCRIPT' => "./#{File.basename(script)}"
    end
    FileUtils.chmod 0755, File.join(tmp_dir, "sh-install")
    sofar = 0.67
    
    @pkgstat.text = "Compute size and compress"
    raw = Shy.du(tmp_dir)
    File.open(tgz_path, 'wb') do |f|
	  pblk = Shy.progress(raw) do |name, perc, left|
	    blk[prog + (perc * prog)]
	  end if blk
	  Shy.czf(f, tmp_dir, &pblk)
    end
    
    @pkgstat.text = "Checksum, install  and copy"
    md5, fsize = Shy.md5sum(tgz_path), File.size(tgz_path)
    File.open(run_path, 'wb') do |f|
	  rewrite f, File.join(DIR, "static", "stubs", "blank.run"),
	   'CRC' => '0000000000', 'MD5' => md5, 'LABEL' => app_name, 'NAME' => name,
	   'SIZE' => fsize, 'RAWSIZE' => (raw / 1024) + 1, 'TIME' => Time.now, 'FULLSIZE' => raw
      File.open(tgz_path, 'rb') do |f2|
	     f.write f2.read(8192) until f2.eof
	  end
    end
    @pkgstat.text = "Done"
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
    blk[1.0] if blk
  end
  
  def repack_exe
      script = @path
      size = File.size(script)
      f = File.open(script, 'rb')
      exe = Binject::EXE.new(File.join(DIR, "static", "stubs", "blank.exe"))
      size += script.length
      exe.inject("SHOES_FILENAME", File.basename(script))
      size += File.size(script)
      exe.inject("SHOES_PAYLOAD", f)
      f2 = open(@work_path)
      @pkgstat.text = "Repack Shoes.exe distribution"

      size += File.size(f2.path)
      f3 = File.open(f2.path, 'rb')
      exe.inject("SHOES_SETUP", f3)

      count, last = 0, 0.0
      exe.save(script.gsub(/\.\w+$/, '') + ".exe") do |len|
        count += len
        prg = count.to_f / size.to_f
        #blk[last = prg] if blk and prg - last > 0.02 and prg < 1.0
      end
        
      f.close
      f2.close
      f3.close
      @pkgstat.text = "Done"
   end

   def repack_osx
      script = @path
      name = File.basename(script).gsub(/\.\w+$/, '')
      app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
      vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
      app_app = "#{app_name}.app"
      vers = [1, 0]

      tmp_dir = File.join(LIB_DIR, "+dmg")
      FileUtils.rm_rf(tmp_dir)
      FileUtils.mkdir_p(tmp_dir)
      FileUtils.cp(File.join(DIR, "static", "stubs", "blank.hfz"),
                   File.join(tmp_dir, "blank.hfz"))
      app_dir = File.join(tmp_dir, app_app)
      res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
      mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
      [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
      FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
      FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
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
  <string>Shoes.icns</string>
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
      File.open(File.join(mac_dir, "#{name}-launch"), 'w') do |f|
        f << <<END
#!/bin/bash
SHOESPATH=/Applications/Shoes.app/Contents/MacOS
APPPATH="${0%/*}"
unset DYLD_LIBRARY_PATH
cd "$APPPATH"
echo "[Pango]" > /tmp/pangorc
echo "ModuleFiles=$SHOESPATH/pango.modules" >> /tmp/pangorc
if [ ! -d /Applications/Shoes.app ]
  then ./cocoa-install
fi
open -a /Applications/Shoes.app "#{File.basename(script)}"
# DYLD_LIBRARY_PATH=$SHOESPATH PANGO_RC_FILE="$APPPATH/pangorc" $SHOESPATH/shoes-bin "#{File.basename(script)}"
END
      end
      FileUtils.cp(script, File.join(mac_dir, File.basename(script)))
      FileUtils.cp(File.join(DIR, "static", "stubs", "cocoa-install"),
        File.join(mac_dir, "cocoa-install"))
      puts "Create DMG"
      dmg = Binject::DMG.new(File.join(tmp_dir, "blank.hfz"), vol_name)
      @pkgstat.text = "reading distribution"
      f2 = open(@work_path)
      dmg.grow(10)
      dmg.inject_file("setup.dmg", f2.path)
      dmg.inject_dir(app_app, app_dir)
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/#{name}-launch")
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/cocoa-install")
      dmg.save(script.gsub(/\.\w+$/, '') + ".dmg") do |perc|
        blk[perc * 0.01] if blk
      end
      FileUtils.rm_rf(tmp_dir)
      @pkgstat.text = 'OSX done'
      blk[1.0] if blk
  end

  
end
