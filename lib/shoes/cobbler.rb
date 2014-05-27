# Cobbler - various Shoes maintenance things

require 'rubygems'
require 'rubygems/dependency_installer'
require 'rubygems/uninstaller'

Shoes.app do
  stack do
    @menu = flow do
      button "Clear Image Cache..." do 
        cachescreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/ 
        button "Jail Break Gems..." do
          jailscreen
        end
        button "Development Tools..." do
          depscreen
        end
      end
      button "Manage Gems..." do 
        gemscreen
      end
      button "Copy Samples..." do
        cp_samples_screen
      end
      button "Quit" do
         exit
      end
    end
    @panel = stack do
      @status = para ""
   end
  end
  
  def jailscreen
    @panel.clear
    @panel.append do
      para "Use directories below for loading gems"
      dirlist = []
      jloc = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}/getoutofjail.card"
      if File.exist? jloc
        open(jloc, 'r') do |f|
         f.each do |l| 
           ln = l.strip
           dirlist << ln if ln.length > 0
         end
        end
      end
      @dirbox = edit_box :width => 600
      @dirbox.text = dirlist.join("\n") if dirlist.length > 0
      flow do 
	      button "Add Gem Directory" do
	        dir = ask_open_folder
	        dirlist << dir if dir
	        @dirbox.text = dirlist.join("\n")
	      end
	      button "Save JailBreak Values" do
	        dirlist = @dirbox.text.split("\n")
	        open(jloc,'w') do |f|
	          dirlist.each {|ln| f.puts ln}
	        end
	        @panel.append {para "Please quit and restart Shoes for changes to take effect"}
	      end
	  end
     
    end
  end
  
  def imgdeletef
  end
  
  def cp_samples_screen
    @panel.clear
    @panel.append do
       para "Copy samples to a directory you can see and edit."
       para "Chose a directory that you want the Samples directory" 
       para "to be created inside of."
       button "Select Directory for a copy of" do
         # OSX is a bit brain dead for ask_save_folder
         if destdir = ask_save_folder() 
           @panel.append do 
             para "Copy #{DIR}/samples/* to #{destdir}/Samples ?"
             button "OK" do
               @panel.append do
                 @lb = edit_box 
               end
               ary = []
               require 'fileutils'
               mkdir_p destdir 
               sampdir = File.join DIR, 'samples'
               cd sampdir do
                 Dir.glob('*').each do |fp|
                   cp fp, destdir
                   ary << fp
                   @lb.text = ary.join("\n")
                 end
               end
             end
           end
         end
       end
    end
  end
  
  def cachescreen
    @panel.clear
    require 'shoes/data'
    require 'shoes/image'
    litems = ['-all-']
    @panel.append do
      para "Select image to delete or all"
      @dblist = list_box :items => litems, :choose =>'-all-'
      #para "DB at: #{LIB_DIR}"
      #para "cache: #{CACHE_DIR}"
      DATABASE.each do |key, value|
        litems << "#{key}"
      end
      @dblist.items = litems
    end
    @panel.append do 
      flow do
        button "Delete From Cache" do
          sel = @dblist.text
          if confirm "Delete #{sel}"
            fdel = @filestoo.checked?
            if sel == '-all-'
              #puts 'delete all'
              DATABASE.each do |k, val| 
                v = val.split('|')
                path = Shoes::image_cache_path v[1], File.extname(k)
                #puts "Deleted #{path}"
                File.delete path
              end
              DATABASE.clear
            else
              #delete single item.
              if fdel
                v = DATABASE[sel].split('|')
                path = Shoes::image_cache_path v[1], File.extname(sel)
                File.delete path
              end
              DATABASE.delete(sel) # block doesn't work as expected
            end
          end
        end
        @filestoo = check; para "Remove file? (I really need disk space)"
      end
    end
  end
  
  # below was copied from shoes.rb, renamed, twerked. 
  def cobblesetup &blk
    require 'shoes/setup'
    script = nil		# because
    puts "cobbler url: #{location}"
    setwin = Shoes::Setup.new(script, &blk)
    puts setwin.inspect
    setwin.close
  end
  
  def geminfo gem
    str = gem
    if gem.kind_of? Gem::Specification
      str = "#{gem.name} #{gem.version} #{gem.summary}\nSee: #{gem.homepage}"
    end
    alert str
  end
  
  def gemremove spec
    if confirm "Really delete gem #{spec.name}"
      begin 
        del = Gem::Uninstaller.new(spec)
        del.remove(spec)
      rescue Exception => e 
        alert e
      end
      gem_refresh_local
    end
  end 
  
  def geminstall spec
    if confirm "Install #{spec.name},#{spec.version} and dependencies?"
      cobblesetup do
        gem spec.name, spec.version
      end
      gem_refresh_local
    end
  end
  

  def gem_refresh_local
    @gemlist.clear
    @gemlist.background white
    # FIXME: deprecated call, returns []
    gemlist =  Gem::Specification.all()
    gemlist.each do |gs| 
      @gemlist.append do 
      flow margin: 5 do
          button 'info', height: 28, width: 50, left_margin: 10 do
             geminfo gs
           end
           button 'delete', height: 28, width: 60, left_margin: 10 do
             gemremove gs
           end
           para "#{gs.name}, #{gs.version}"
         end
       end
    end
  end
  
  def gemsearch str
    installer = Gem::DependencyInstaller.new
    begin
      poss_gems = installer.find_spec_by_name_and_version(str, Gem::Requirement.default)
    rescue Gem::SpecificGemNotFoundException => e
      @gemlist.append {para "not found"}
      return
    end
    poss_gems.each_spec do |g|   
      @gemlist.append do
        flow margin: 5 do
          button 'info', height: 28, width: 50, left_margin: 10 do
            geminfo g
          end
          button 'install', height: 28, width: 60, left_margin: 10 do
            geminstall g
          end
          para "#{g.name},#{g.version}"
        end
      end
    end
  end
   
  def gemscreen
    @panel.clear
    @panel.append do
      para "Manage Gems"
      flow do
        button 'Show Local' do
          @gemlist.clear
          @gemlist.background white
          gem_refresh_local
        end
        @searchphrase = edit_line 
        button 'Search Remote' do
          @gemlist.clear
          @gemlist.background "#EEE".."#9AA"
          gemsearch @searchphrase.text
        end
      end
      @gemlist = stack width: 0.90, left_margin: 5, top_margin: 5 do     
        background white  
      end
    end
  end
  
  def depscreen
    @panel.clear
    @panel.append do
      para 'development tools not implemented yet'
      case RUBY_PLATFORM 
      when  /linux/
        # lsb_release not in all Linux distro's
        para "lsb: #{`lsb_release -i -s`}"
      when  /darwin/
        para "OSX Dependencies"
      else
        para "Windows Dependecies"
      end
    end
   end 
    
end # App


