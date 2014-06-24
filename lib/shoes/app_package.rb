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
    @info_panel.append do
      case @work_path
      when /\.run$/
        Thread.new do
          @pkgstat = inscription "Linux repack #{@path} for#{@work_path}"
          @pkgbar = progress :width => 1.0, :height => 14 
          repack_linux
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
  
  def repack_linux  #(script, opt, &blk)
    script = @path
    sofar, stage = 0.0, 1.0 
    blk = proc do |frac|
      @pkgbar.style(:width => sofar + (frac * stage))
    end
    name = File.basename(script).gsub(/\.\w+$/, '')
    app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
    run_path = script.gsub(/\.\w+$/, '') + ".run"
    tgz_path = script.gsub(/\.\w+$/, '') + ".tgz"
    tmp_dir = File.join(LIB_DIR, "+run")
    FileUtils.mkdir_p(tmp_dir)
    # pkgf = pkg("linux", opt)
    pkgf = open(@work_path)
    prog = 1.0
    if pkgf
	  size = Shy.hrun(pkgf)
	  pblk = Shy.progress(size) do |name, perc, left|
	    blk[perc * 0.5]
	  end if blk
	  Shy.xzf(pkgf, tmp_dir, &pblk)
	  puts "extract complete"
	  prog -= 0.5
    end

    FileUtils.cp(script, File.join(tmp_dir, File.basename(script)))
    puts "rewrite sh-install"
    File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
	  rewrite a, File.join(DIR, "static", "stubs", "sh-install"),
	    'SCRIPT' => "./#{File.basename(script)}"
    end
    FileUtils.chmod 0755, File.join(tmp_dir, "sh-install")
    puts "start du"
    raw = Shy.du(tmp_dir)
    File.open(tgz_path, 'wb') do |f|
	  pblk = Shy.progress(raw) do |name, perc, left|
	    blk[prog + (perc * prog)]
	  end if blk
	  Shy.czf(f, tmp_dir, &pblk)
    end
    puts "checksum"
    md5, fsize = Shy.md5sum(tgz_path), File.size(tgz_path)
    File.open(run_path, 'wb') do |f|
	rewrite f, File.join(DIR, "static", "stubs", "blank.run"),
	  'CRC' => '0000000000', 'MD5' => md5, 'LABEL' => app_name, 'NAME' => name,
	  'SIZE' => fsize, 'RAWSIZE' => (raw / 1024) + 1, 'TIME' => Time.now, 'FULLSIZE' => raw
	File.open(tgz_path, 'rb') do |f2|
	  f.write f2.read(8192) until f2.eof
	  end
    end
    puts "closing up"
    FileUtils.chmod 0755, run_path
    FileUtils.rm_rf(tgz_path)
    FileUtils.rm_rf(tmp_dir)
    blk[1.0] if blk
  end

  
end
