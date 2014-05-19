# Cobbler - various Shoes maintenance things


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
        
        button "Manage Gems..." do 
          gemscreen
        end
        button "Development Tools..." do
          depscreen
        end
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
  
  def gemscreen
    @panel.clear
    @panel.append do
       para "gem screen - not implemented yet"
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


