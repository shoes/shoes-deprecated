# The 'New' packager for Shoes 3.2
require 'shoes/shy'
require 'shoes/winject'
require 'open-uri'
require 'rubygems/package'
require 'zlib'
require 'shoes/packshoes'

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
		      #puts edit2.text
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
			              #puts "Shy is #{$script_path}"
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
     para "Include a full copy of Shoes 8 to 15 MB or download if needed?"
      flow do
        @inclcheck = check; para "Shoes will be included with my app."
      end
      @inclcheck.checked = @options['inclshoes'] = false
      para "Advanced installer -- Must be a .shy (directory) to package"
      flow do
        @defadvopts = check  do
          @advpanel.show if @defadvopts.checked?
          @advpanel.hide if !@defadvopts.checked?
          @options['advopts'] = @defadvopts.checked?
        end
        para "I want advanced options"
      end
      @advpanel = stack :hidden => true do
       flow do
         para "I have my own install script  "
         button "Select script" do
           cf = ask_open_file
           @options['custominstaller'] = cf if cf
         end
       end
       flow do
        @options['expandshy'] = false
        @expandshy = check do 
           @options['expandshy'] = @expandshy.checked?
         end
         para "Expand shy in users directory"
       end
       #flow do
       #   check; para "I have gems to be installed"
       #end
       para "Add app icons - Always add a .png"
       flow do
          button "Windows .ico file" do
            wicf = ask_open_file
            @options['ico'] = wicf if wicf
          end
          button "OSX .icns file" do
            micf = ask_open_file
            @options['icns'] = micf if micf
          end
          button "Linux .png file" do
            licf = ask_open_file
            @options['png'] = licf if licf
          end
        end
      end
    end
    @menu_panel = stack do
      para "Package one of the options below."
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
                      if @inclcheck.checked?
                        platform_download flds[2], flds[0], flds[1]
                      else
                        platform_dnlif flds[2]
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
    #@running_windows = RUBY_PLATFORM =~ /mingw/
    @options['advopts'] = @defadvopts.checked?
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
    @options['advopts'] = @defadvopts.checked?
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
    @options['app'] = $script_path
    @options['arch'] = arch
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/#{arch}.rb"
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    PackShoes.dnlif_linux @options
    @pkgstat = inscription "Done packaging #{$script_path} for Linux #{arch}"
  end
  
  def dnlif_exe
    @options['app'] = $script_path
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/win32.rb"
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    PackShoes.dnlif_exe @options
    @pkgstat = inscription "Done packaging #{$script_path} for Windows"
 end

  def dnlif_osx
    @options['app'] = $script_path
    @options['arch'] = 'osx'
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/osx.rb"
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    @options['shoesruby'] = SHOES_RUBY_ARCH
    PackShoes.dnlif_osx @options
    @pkgstat = inscription "Done packaging #{$script_path} for OSX" 
  end
  
# ===== full download and package ===  

  def repack_linux  arch
    @options['app'] = $script_path
    @options['arch'] = arch
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/#{arch}.rb"
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    @options['shoesdist'] = @work_path
    PackShoes.repack_linux @options do |msg|
      @pkgstat.text = msg
    end
    @pkgstat = inscription "Done packaging #{$script_path} for Linux #{arch}"
  end
  
  def repack_exe
    @options['app'] = $script_path
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/win32.rb"
    @options['shoesdist'] = @work_path
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    PackShoes.repack_exe @options do |msg|
      @pkgstat.text = msg
    end
    @pkgstat = inscription "Done packaging #{$script_path} for Windows"
   end

  def repack_osx
    @options['app'] = $script_path
    @options['arch'] = 'osx'
    @options['dnlhost'] = @dnlhost
    @options['dnlpath'] = "/public/select/osx.rb"
    @options['shoesdist'] = @work_path
    @options['packtmp'] = LIB_DIR
    @options['relname'] = Shoes::RELEASE_NAME
    @options['shoesruby'] = SHOES_RUBY_ARCH
    PackShoes.repack_osx @options do |msg|
      @pkgstat.text = msg
    end
    @pkgstat = inscription "Done packaging #{$script_path} for OSX" 
  end

end
