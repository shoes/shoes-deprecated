# The 'New' packager for Shoes 3.2
Shoes.app do
  # get the urls or default
  @shoes_home = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}"
  @selurl = "#{@shoes_home}/package/selector.url"
  if File.exists? @selurl
    File.open(@selurl,"r") do |f|
      puts "using cache"
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
      puts "using dnl cache #{@dnl_site}"
    end
  else
    @dnl_site = 'http://shoes.mvmanila.com/public/shoes'
  end
  stack do
    para "Package A Shoes Application"
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
                      platform_process flds[2]
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
    @info_panel = stack do
    end
  end
  
  # FIXME: Assumes lines are sorted by filename at server
  # Should check version number and timestamp and only show the latest
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
  
  #here's where the fun is. Processing one item.  Download it, pull it
  # apart, put the app in and put it back together
  def platform_process fname
    @info_panel.append do
      para "Downloading #{@dnl_site}/#{fname}"
    end
  end
end
