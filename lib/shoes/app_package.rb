# The 'New' packager for Shoes 3.2
require 'shoes/shy'
require 'shoes/winject'
require 'open-uri'
require 'rubygems/package'
require 'zlib'

# I'm going to use a $global for the name of script (or shy).
# because @path is not the same @path in a different Shoes Window.
$script_path = ""
Shoes.app height: 600 do
  # get the urls or default
  @shoes_home = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}"
  @selurl = "#{@shoes_home}/package/selector.url"
  @options = {}
  @options['inclshoes'] = false
  if File.exists? @selurl
    File.open(@selurl,"r") do |f|
      @home_site = f.read
    end
  else
    @home_site = 'http://shoes.mvmanila.com/public/select/pkg.rb'
  end
  dnlurl = "#{@shoes_home}/package/download.url"
  if File.exists? dnlurl
    File.open(dnlurl,"r") do |f|
      @dnl_site = f.read.strip
      @dnl_site.gsub!(/\/$/,'')
    end
  else
    @dnl_site = 'http://shoes.mvmanila.com/public/shoes'
  end
  dnluri = URI.parse(@dnl_site)
  @dnlhost = dnluri.host
  @dnlpath = dnluri.path
  
  stack do
    para "Package A Shoes Application"
    selt = proc { @sel1.toggle; @sel2.toggle }
    $script_path = ""
    @shy_path = nil
    @sel1 =
      flow do
	    para "File to package:"
	    inscription " (or a ", link("directory", &selt), ")"
	    edit1 = edit_line :width => -120
	    @bb = button "Browse...", :width => 100 do
		  $script_path = edit1.text = ask_open_file
	    end
	  end
    @sel2 =
	  flow :hidden => true do
	    para "Directory: (select the startup script) in the directory"
	    inscription " (or a ", link("only a single file", &selt), ")"
	    edit2 = edit_line :width => -120
	    @bf = button "Start script", :width => 100 do
	      # need to create a shy 
		  $script_path = edit2.text = ask_open_file
		  if edit2.text != ""
		    window do
		      puts edit2.text
			  launch_script = File.expand_path(edit2.text)
			  top_dir = File.dirname(edit2.text)
			  launch_script = File.basename(edit2.text)
	          shy_name = "#{top_dir}.shy"
	          background white
		      stack do
		        para "Package #{top_dir} into #{shy_name}"
		        para "Enter something below that matters to you. "
		        fields = {}
		        for label, name in [["Project Name", "name"],
		                            ["Version", "version"],
		                            ["Your Name", "creator"]]
		          flow :width => 1.0 do
		            para "#{label}: "
		            fields[name] = edit_line ''
		          end
		        end
		        flow do
			        button "Cancel" do
			          close
			        end
			        button "Build .shy" do
			          shy_desc = Shy.new
			          for name in fields.keys
			            shy_desc.send("#{name}=".intern, fields[name].text)
			          end
			          # TODO create a launch_options in shy_desc
			          shy_desc.launch = launch_script
			          Shy.c(shy_name, shy_desc, top_dir)
			          clear
			          background white
			          stack do
			            para "Built #{shy_name}"
			            button "Ok" do
			              $script_path = edit2.text = shy_name
			              puts "Shy is #{$script_path}"
			              close
			            end
			          end
			        end
		        end
		      end
 
		    end
		  end
	    end
	  end
    @options_panel = stack do
      flow do
        @dnlradio = radio :dnl; para "Shoes will be downloaded if needed."
        @dnlradio.checked = true;  # OSX requires - a bug
      end
      flow do
        @inclradio = radio :dnl; para "Shoes will be included with my app."
      end
      @inclradio.checked = @options['inclshoes']
      # comment out 3.2.18 options
      para "Advanced installer -- CAUTION -- may not work everywhere"
      flow do
        @noadvopts = radio :advopts; para "No thanks." 
        @defadvopts = radio :advopts do
          @advpanel.show if @defadvopts.checked?
          @advpanel.hide if !@defadvopts.checked?
        end
        para "I want advanced options"
      end
      @advpanel = stack :hidden => true do
        flow do
          para "I have my own install script  "
          button "Select script"
        end
        flow do
          @expandshy = check do 
            @options['expandshy'] = @expandshy.checked?
          end
          para "Expand shy in users directory"
        end
      #  flow do
      #    check; para "I have gems to be installed"
      #  end
      #  flow do
      #    check; para "I have icons for Windows, OSX and Linux"
      #  end
      end
    end
    @menu_panel = stack do
      flow do 
        button 'Select Architecture' do
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
                      if @dnlradio.checked?
                        platform_dnlif flds[2]
                      else
                        platform_download flds[2], flds[0], flds[1]
                      end
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
  
  # Assumes lines are sorted by filename on server (cgi select/pkg.rb)
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
      when /\.run$/
        return  #  short circuit - ignore .runs in 3.2.15+ 
      when /osx\-.*\.tgz$/
        @platforms['OSX'] = ln
     when /armhf\.run$/
        @platforms['Linux_Raspberry'] = ln
      when /i686\.run$/
        @platforms['Linux_i686'] = ln
      when /x86_64\.run$/ 
        @platforms['Linux_x86_64'] = ln
      when /armhf\.install$/
        @platforms['Linux_Raspberry'] = ln
      when /i686\.install$/
        @platforms['Linux_i686'] = ln
      when /x86_64\.install$/ 
        @platforms['Linux_x86_64'] = ln
      when /tar\.gz$/
        tarball = ln
      else
        #puts "failed match #{ln}"
    end
    return
  end
  
  #here's where the fun begins. Processing one item.  Download it, pull it
  # apart, put the app in and put it back together. Or something like that.
  def platform_download fname, szMB, timestamp
    if $script_path == '' 
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
    @running_windows = RUBY_PLATFORM =~ /mingw/
    @info_panel_clear
    @info_panel.append do
      case @work_path
      when /\.run$/
       Thread.new do
          @pkgstat = inscription "Linux repack #{$script_path} for#{@work_path}"
          #@pkgbar = progress :width => 1.0, :height => 14 
          arch = @work_path[/(\w+)\.run$/] 
          arch.gsub!('.run','')
          repack_linux arch
        end
      when /\.install$/
       # Shoes 3.2.15 and later uses .install
       Thread.new do
          @pkgstat = inscription "Linux repack #{$script_path} for#{@work_path}"
          #@pkgbar = progress :width => 1.0, :height => 14 
          arch = @work_path[/(\w+)\.install$/] 
          arch.gsub!('.install','')
          @options['arch'] = arch
          repack_linux arch
        end
      when /.exe$/
        Thread.new do
          @pkgstat = inscription "Windows repack #{$script_path} for#{@work_path}"
          repack_exe
        end
      when /osx\-.*.tgz$/
        Thread.new do
          @pkgstat = inscription "OSX repack #{$script_path} for#{@work_path}"
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

# ==== Download If Needed  (dnlif) packaging ===
  def platform_dnlif arch
    # No need to thread - this is simple and fast. just call the {platform}_dnilf
    case arch
    when /\.exe$/
      dnlif_exe
    when /\.install$/
      dnlif_linux arch
    when /\.tgz$/
       dnlif_osx
    else
      alert "Can't do dnlif #{arch}"
    end
  end
  
  def dnlif_linux installname
    # pick apart the installname arg to tease out which linux to put into
    # the packed install.sh
    arch = installname[/(\w+)\.install$/]
    arch.gsub!(/\.install/,"")
    #alert "Pack for #{arch} #{installname} #{$script_path}"
    script = custom_installer $script_path
    puts "script is #{script} #{$script_path}"
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tar_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tar"
    tmp_dir = File.join(LIB_DIR, "+run")
    FileUtils.mkdir_p(tmp_dir)
    # rewrite the static/stubs/sh-install.tmpl to point to the download
    # website, path and script/shy
    File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "sh-install.tmpl"),
	    'HOST' => @dnlhost, 'ARCH' => arch, 
	    'PATH' => "/public/select/#{arch}.rb",
	    'RELNAME' => Shoes::RELEASE_NAME,
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
    @pkgstat = inscription "Done packaging #{$script_path} for Linux #{arch}"
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
   
  end
  
  def dnlif_exe
    script = custom_installer $script_path
    debug "win script = #{script}"
    size = File.size(script)
    f = File.open(script, 'rb')
    inf = File.join(DIR, "static", "stubs", 'shoes-stub.exe')
    begin
      exe = Winject::EXE.new(inf)
      exe.inject_string(Winject::EXE::SHOES_APP_NAME, File.basename(script))
      exe.inject_file(Winject::EXE::SHOES_APP_CONTENT, f.read)
      exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_SITE, @dnlhost)
      exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_PATH, "/public/select/win32.rb")
      exe.save(script.gsub(/\.\w+$/, '') + ".exe") 
    rescue StandardError => e
        error "Failed to create Winject::Exe #{e}"
        error e.backtrace.join("\n")
        puts "Failed to create Winject::EXE #{e}"
        puts e.backtrace.join("\n")
    end
        
    f.close
    @pkgstat = inscription "Done packaging #{$script_path} for Windows"
  end

  def dnlif_osx
    script = custom_installer $script_path
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    #vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
    tar_path = script.gsub(/\.\w+$/, '') + "-osx.tar"
    tgz_path = script.gsub(/\.\w+$/, '') + "-osx.tgz"
    app_app = "#{app_name}.app"
    vers = [2, 0]

    tmp_dir = File.join(LIB_DIR, "+dmg")
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # Produce a tgz that when the user expands it is a 'Myapp.App'
    # which is a directory tree with all the osx stuff. Info.plist
    # will run a bash script (Contents/MacOS/osx-app-install) which 
    # checks if Shoes is already installed in /Applications
    # if not it will download shoes, and install it
    # Then the script/shy is run. Actually a MyApp-launch bash script
    # is whats run - it starts Shoes and pass the Myapp.rb or MyApp.shy on
    # the commandline. 
    # Similar to what is done for Linux, but with extra osx flavoring.
  
    @tarmodes = {}
 
    app_dir = File.join(tmp_dir, app_app)

    res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
    mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
    [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
    FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
    FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
    # make cache entries for the files above just to keep the console
    # messages away. 
    @tarmodes["#{app_app}/Contents/Resources/Shoes.icns"] = 0644
    @tarmodes["#{app_app}/Shoes.icns"] = 0644
    # more permissions cache entries 
    ["#{app_app}", "#{app_app}/Contents", "#{app_app}/Contents/Resources",
     "#{app_app}/Contents/MacOS" ].each {|d| @tarmodes[d] = 0755}
    ["#{app_app}/Contents/PkgInfo", "#{app_app}/Contents/version.plist",
     "#{app_app}/Contents/Info.plist" ].each {|f| @tarmodes[f] = 0644}
    File.open(File.join(app_dir, "Contents", "PkgInfo"), 'w') do |f|
      f << "APPL????"
    end
    # info.plist with substitutions
    @dnlsel = '/public/select/osx.rb'
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
DYLD_LIBRARY_PATH="$APPPATH" PANGO_RC_FILE="$APPPATH/pangorc" SHOES_RUBY_ARCH="#{SHOES_RUBY_ARCH}" ./shoes-bin "$this_dir/#{File.basename(script)}"
END
    end
    ls = File.join(mac_dir, "#{name}-launch")
    chmod 0755, ls
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
	    'HOST' => "http://#{@dnlhost}", 'ARCH' => 'osx', 
	    'PATH' => "/public/select/osx.rb",
	    'RELNAME' => Shoes::RELEASE_NAME,
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
    @pkgstat = inscription "Done packaging #{$script_path} for OSX"
 end
  
# ===== full download and package ===  
  def repack_linux  arch
    script = custom_installer $script_path
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tar_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tar"
    tmp_dir = File.join(LIB_DIR, "+run")
    FileUtils.mkdir_p(tmp_dir)
    pkgf = open(@work_path,'rb')  # downloaded/cached .run/.install
    @pkgstat.text = "Expanding #{arch} distribution. Patience is needed"
    # skip to the tar contents in the .run
	size = Shy.hrun(pkgf)
	# Copy the rest to a new file
	wk_dir = File.join(LIB_DIR, "+tmp")
    FileUtils.mkdir_p(wk_dir)
	wkf = open(File.join(wk_dir,"run.gz"),'wb')
	buff = ''
	while pkgf.read(32768, buff) != nil do
	   wkf.write(buff) 
    end
	wkf.close
	@pkgstat.text = "Start extract"
	@tarmodes = {}
	fastxzf(wkf, tmp_dir, @tarmodes)
    FileUtils.rm_rf(wk_dir)
    @pkgstat.text = "Copy script and stubs"
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
    
 
    @pkgstat.text = "Compute size and compress"
    raw = Shy.du(tmp_dir)
    
    #File.open(tgz_path, 'wb') do |f|
 	  #Shy.czf(f, tmp_dir)
    #end
    
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
    @pkgstat.text = "Done packing Linux"
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
  end
  
  def repack_exe
      script = custom_installer $script_path
      size = File.size(script)
      f = File.open(script, 'rb')
      begin
        #exe = Winject::EXE.new(File.join(DIR, "static", "stubs", "blank.exe"))
        exe = Winject::EXE.new(File.join(DIR, "static", "stubs", "shoes-stub.exe"))
        exe.inject_string(Winject::EXE::SHOES_APP_NAME, File.basename(script))
        exe.inject_file(Winject::EXE::SHOES_APP_CONTENT, f.read)
        exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_SITE, @dnlhost)
        exe.inject_string(Winject::EXE::SHOES_DOWNLOAD_PATH, "/public/select/win32.rb")
        f2 = File.open(@work_path,'rb')
        @pkgstat.text = "Repack Shoes.exe #{@work_path} distribution"
        exe.inject_file(Winject::EXE::SHOES_SYS_SETUP, f2.read)
        exe.save(script.gsub(/\.\w+$/, '') + ".exe") do |len|
      end
      rescue StandardError => e
        puts "Failed to create Winject::EXE #{e}"
      end
        
      f.close
      f2.close
      @pkgstat.text = "Done packaging Windows"
   end

  def repack_osx
    script = custom_installer $script_path
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    #vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
    tar_path = script.gsub(/\.\w+$/, '') + "-osx.tar"
    tgz_path = script.gsub(/\.\w+$/, '') + "-osx.tgz"
    app_app = "#{app_name}.app"
    vers = [1, 0]

    tmp_dir = File.join(LIB_DIR, "+dmg")
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # expand download into ~/.shoes/+dmg/Shoes.app
    pkgf = open(@work_path)
    @pkgstat.text = "Expanding OSX distribution. Patience is needed"
	#Shy.xzf(pkgf, tmp_dir)   
	@tarmodes = {}
	fastxzf(pkgf, tmp_dir, @tarmodes, app_app)
	# debug 
	# @tarmodes.each_key {|k| puts "entry #{k} = #{@tarmodes[k]}" }
	# DMG stuff in case I need it:
    #  FileUtils.cp(File.join(DIR, "static", "stubs", "blank.hfz"),
    #              File.join(tmp_dir, "blank.hfz"))
    app_dir = File.join(tmp_dir, app_app)
    # rename Shoes.app to app_dir
    chdir tmp_dir do
      mv "Shoes.app", app_app
    end
    res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
    mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
    [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
    FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
    FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
    # make cache entries for two files above just to keep the consoles
    # messages away. 
    @tarmodes["#{app_app}/Contents/Resources/Shoes.icns"] = 0644
    @tarmodes["#{app_app}/Shoes.icns"] = 0644
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
DYLD_LIBRARY_PATH="$APPPATH" PANGO_RC_FILE="$APPPATH/pangorc" SHOES_RUBY_ARCH="#{SHOES_RUBY_ARCH}" ./shoes-bin "#{File.basename(script)}"
END
    end
    ls = File.join(mac_dir, "#{name}-launch")
    chmod 0755, ls
    @tarmodes["#{app_app}/Contents/MacOS/#{name}-launch"] = 0755
    FileUtils.cp(script, File.join(mac_dir, File.basename(script)))
    @tarmodes["#{app_app}/Contents/MacOS/#{File.basename(script)}"] = 0644
    #FileUtils.cp(File.join(DIR, "static", "stubs", "cocoa-install"),
    #  File.join(mac_dir, "cocoa-install"))
    @pkgstat.text = "Creating new archive"
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
    @pkgstat.text = 'OSX done'
  end

  def fastxzf infile, outdir, modes = {}, osx = ''
	#from blog post http://dracoater.blogspot.com/2013/10/extracting-files-from-targz-with-ruby.html
	# modified by Cecil Coupe - Jun 27+, 2014. Thanks to Juri Timošin   
	
	tar_longlink = '././@LongLink'
	begin
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
	    @pkgstat.text = hashname
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
	      @pkgstat.text = "Symlink #{entry.header.linkname}"
	      File.symlink entry.header.linkname, dest
	    end
	    dest = nil
	  end
	end
	rescue StandardError => e
	   error e
	end
  end
  
  def fastcf outf, indir, modes
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
  
  def custom_installer defshy
    if @options['expandshy'] != true
        return defshy
    end
    if ! defshy[/\.shy$/] then
        alert "Must be a shy"
        return defshy
    end
    appname = File.basename(defshy,".shy")
    shydir = File.dirname(defshy)
    tmp_dir = File.join(LIB_DIR, "+shy", appname)
    FileUtils.rm_rf(tmp_dir)
    FileUtils.mkdir_p(tmp_dir)
    # copy shy to tmp - TODO: Copy User Script here
    FileUtils.cp(defshy, tmp_dir)
    # copy app-install.tmpl with rewrite
    File.open(File.join(tmp_dir, "#{appname}-install.rb"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "app-install.tmpl"),
	    'SHYFILE' => "#{defshy}"
    end
    # TODO: Copy icons.zip and gems.zip here.
    # Create a shy header for the installer.
    shy_desc = Shy.new
    shy_desc.launch = "#{appname}-install.rb"
    shy_desc.name = appname
    shy_desc.version = "1.0"
    shy_desc.creator = "Shoes"
    # Create the new shy. Grrr -- read shy.rb
    new_shypath = File.join(LIB_DIR, '+shy')
    Dir.chdir(new_shypath) do
      Shy.c(appname+'.shy', shy_desc, tmp_dir)
    end
    alert "Check your Directories #{new_shypath}\n amd #{defshy}"
    # write over the incoming shy with the new one
    FileUtils.cp File.join(new_shypath,"#{appname}.shy"), defshy
    # delete the +shy temp stuff
    return defshy
  end

end
