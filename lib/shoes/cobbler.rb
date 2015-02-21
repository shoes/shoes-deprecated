# Cobbler - various Shoes maintenance things
# CJC: the gem handling is kind of ugly. Someone should make it pretty.
require 'rubygems'
require 'rubygems/dependency_installer'
require 'rubygems/uninstaller'

module Gem
  if Shoes::RELEASE_TYPE =~ /TIGHT/
    @ruby = (File.join(RbConfig::CONFIG['bindir'], 'shoes') + RbConfig::CONFIG['EXEEXT']).
          sub(/.*\s.*/m, '"\&"') + " --ruby"
  end
end
class << Gem::Ext::ExtConfBuilder
  alias_method :make__, :make
  def make(dest_path, results)
    raise unless File.exist?('Makefile')
    mf = File.read('Makefile')
    mf = mf.gsub(/^INSTALL\s*=\s*.*$/, "INSTALL = $(RUBY) -run -e install -- -vp")
    mf = mf.gsub(/^INSTALL_PROG\s*=\s*.*$/, "INSTALL_PROG = $(INSTALL) -m 0755")
    mf = mf.gsub(/^INSTALL_DATA\s*=\s*.*$/, "INSTALL_DATA = $(INSTALL) -m 0644")
    File.open('Makefile', 'wb') {|f| f.print mf}
    make__(dest_path, results)
  end
end

class Gem::CobblerFace
  class DownloadReporter  #ProgressReporter
    attr_reader :count

    def initialize(prog, status, size, initial_message,
                   terminal_message = "complete")
      @prog = prog
      (@status = status).replace initial_message
      @total = size
      @count = 0.0
    end

    def updated(message)
      @count += 1.0
      @prog.fraction = (@count / @total.to_f) * 0.5
    end
    
    def fetch(filename, len)
      @total = len
    end
    
    def update(len)
      @prog.fraction = len.to_f / @total.to_f
    end
   
    def done
    end
  end
  
  # init CobblerFace
  def initialize bar, statline
    @prog = bar
    @status = statline
    #@status, @prog, = app.slot.contents[-1].contents
    
  end
  def title msg
    @status.text =  msg
  end
  def progress count, total
    @prog.fraction = count.to_f / total.to_f
    #$fraction = count.to_f / total.to_f
  end
  def ask_yes_no msg
    Kernel.confirm(msg)
  end
  def ask msg
    Kernel.ask(msg)
  end
  
  def error msg, e
    @status =  link("Error") { Shoes.show_log }, " ", msg
  end
  
  def say msg
    @status.text =  msg
  end
  
  def alert msg, quiz=nil
    ask(quiz) if quiz
  end
  
  def download_reporter(*args)
    DownloadReporter.new(@prog, @status, 0, 'Downloading')
  end
  
  def method_missing(*args)
    p args
    nil
  end
end

# UI class for delete does nothing - on purpose. Swallows 'success' msg
class Gem::CobblerDelFace
 def initialize 
 end
 def say msg
   #puts "CmdFace: say: #{msg}"
 end
end

Shoes.app do
  @shoes_home = File.join(LIB_DIR, Shoes::RELEASE_NAME)
  stack do
    @menu = flow do
      button "Shoes Info.." do
        infoscreen
      end
      button "Clear Image Cache..." do 
        cachescreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/ 
        button "Jail Break Gems..." do
          jailscreen
        end
      end
      button "Manage Gems..." do 
        gemscreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/ 
        button "Development Tools..." do
          depscreen
        end
      end
      button "Copy Samples..." do
        cp_samples_screen
      end
      button "Packager URLs..." do
        pack_screen
      end
      button "Quit" do
         exit
      end
    end
    @panel = stack do
      @status = para ""
   end
  end
  
  def  pack_screen 
    @panel.clear
    @panel.append do
      flow do
        para "CGI selector  "
        sel = edit_line "http://shoes.mvmanila.com/public/select/pkg.rb", width: 400
        button "Update" do
          mkdir_p "#{@shoes_home}/package"
          File.open("#{@shoes_home}/package/selector.url", 'w') do |f|
            f.write sel.text
          end
        end
      end
      flow do
        para "Download  "
        dnl = edit_line "http://shoes.mvmanila.com/public/shoes", width: 400
        button "Update" do
          mkdir_p "#{@shoes_home}/package"
            File.open("#{@shoes_home}/package/download.url", 'w') do |f|
            f.write dnl.text
          end
        end
      end
    end
  end
  
  def infoscreen
    @panel.clear
    @panel.append do
      para "Ruby Version: #{RUBY_VERSION} on #{RUBY_PLATFORM}"
      para "Shoes Release: #{Shoes::RELEASE_NAME}   #{Shoes::VERSION_NUMBER}  r#{Shoes::VERSION_REVISION}"
      para "    built on #{Shoes::VERSION_DATE}"
      para "    Fit: #{Shoes::RELEASE_TYPE}"
      para "Gems Version #{Gem::RubyGemsVersion}"
      para "Shoes Exe Directory: #{DIR}"
      para "Shoes Home: #{@shoes_home}"
      para "LIB_DIR: #{LIB_DIR}"
    end
  end
  
  def jailscreen
    @panel.clear
    @panel.append do
      para "Use directories below for loading gems"
      dirlist = []
      #jloc = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}/getoutofjail.card"
      jloc = File.join(LIB_DIR, Shoes::RELEASE_NAME, 'getoutofjail.card')
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

  def gem_reset
    Gem.use_paths(GEM_DIR, [GEM_DIR, GEM_CENTRAL_DIR])
    Gem.refresh
  end
  
 
  def gem_install_one spec
    # setup Gem download ui
    ui = Gem::DefaultUserInteraction.ui = Gem::CobblerFace.new(@progbar, @status)
    ui.title "Installing #{spec.name} #{spec.version}"
    installer = Gem::DependencyInstaller.new
    begin
      installer.install(spec.name, spec.version)
      gem_reset
      spec.activate
      ui.say "Finished installing #{spec.name}"
    rescue Object => e
       puts "Fail #{e}"
       @status.replace link("Error") { Shoes.show_log }, " ", e
       #ui.error "while installing #{spec.name}", e
       #raise e
    end
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
        Gem::DefaultUserInteraction.ui = Gem::CobblerDelFace.new()
        del = Gem::Uninstaller.new(spec)
        del.remove(spec)
      rescue Exception => e 
        alert e
      end
      gem_refresh_local
    end
  end 
  
  def geminstall spec
   @gemlist.append do 
     @progbar = progress width: 0.9, margin_left: 0.1, height: 20
     @progbar.fraction = 0.5
     @status = inscription 'Initialize', align: :center
   end
   if confirm "Install #{spec.name},#{spec.version} and dependencies?"
      @thread = Thread.new do 
        gem_install_one spec
        #gem_refresh_local  # not sure I want this UI wise.
      end
    end
  end
  
  def gemloadtest spec
    begin 
      require spec.name
    rescue Exception => e
      alert e
    end
    alert "Loaded"
  end
  

  def gem_refresh_local
    @gemlist.clear
    @gemlist.background white
    # FIXME: deprecated call, returns []
    #gemlist =  Gem::Specification._all()
    #gemlist.each do |gs| 
    Gem::Specification.each do |gs|
      @gemlist.append do 
      flow margin: 5 do
          button 'info', height: 28, width: 50, left_margin: 10 do
             geminfo gs
           end
           button 'delete', height: 28, width: 60, left_margin: 10 do
             gemremove gs
           end
           button 'load test', height: 28, width: 60, left_margin: 10 do
             gemloadtest gs
           end
           para "#{gs.name}, #{gs.version}"
         end
       end
    end
  end
  
  def gemsearch str
    installer = Gem::DependencyInstaller.new domain: :remote
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
          button 'install', height: 28, width: 62, left_margin: 10 do
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


