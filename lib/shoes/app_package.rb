# The 'New' packager for Shoes 3.2
require 'shoes/shy'
require 'winject'
require 'open-uri'
require 'rubygems/package'
require 'zlib'

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
	    end
	  end
    @sel2 =
	  flow :hidden => true do
	    para "Directory: (select the startup script) in the directory"
	    inscription " (or a ", link("only a single file", &selt), ")"
	    edit2 = edit_line :width => -120
	    @bf = button "Start script", :width => 100 do
	      # need to create a shy 
		  @path = edit2.text = ask_open_file
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
		        para "Behaviour when user runs/installs. Untested."
		        flow { check; para "Create directory in users home?" }
		        flow { check; para "Install in users menus?" } 
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
			              @path = edit2.text = shy_name
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
    @running_windows = RUBY_PLATFORM =~ /mingw/
    @info_panel_clear
    @info_panel.append do
      case @work_path
      when /\.run$/
       Thread.new do
          @pkgstat = inscription "Linux repack #{@path} for#{@work_path}"
          #@pkgbar = progress :width => 1.0, :height => 14 
          arch = @work_path[/(\w+)\.run$/] 
          arch.gsub!('.run','')
          repack_linux arch
        end
      when /\.install$/
       # Shoes 3.2.15 and later uses .install
       Thread.new do
          @pkgstat = inscription "Linux repack #{@path} for#{@work_path}"
          #@pkgbar = progress :width => 1.0, :height => 14 
          arch = @work_path[/(\w+)\.install$/] 
          arch.gsub!('.install','')
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
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + "-#{arch}.run"
    tgz_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tgz"
    tar_path = script.gsub(/\.\w+$/, '') + "-#{arch}.tar"
    tmp_dir = File.join(LIB_DIR, "+run")
    FileUtils.mkdir_p(tmp_dir)
    pkgf = open(@work_path,'rb')  # downloaded/cached .run
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
      script = @path
      size = File.size(script)
      f = File.open(script, 'rb')
      begin
        exe = Winject::EXE.new(File.join(DIR, "static", "stubs", "blank.exe"))
      rescue StandardError => e
        puts "Failed to create Winject::EXE #{e}"
      end
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
      @pkgstat.text = "Done packaging Windows"
   end

  def repack_osx
    script = @path
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

end
