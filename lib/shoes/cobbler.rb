# Cobbler - various Shoes maintenance things
# CJC: the gem handling is kind of ugly. Someone should make it pretty.
require 'rubygems'
require 'rubygems/dependency_installer'
require 'rubygems/uninstaller'
require 'rubygems/package'

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

Shoes.app :title => "Shoes Cobbler" do
  @shoes_home = File.join(LIB_DIR, Shoes::RELEASE_NAME)
  stack do
    @menu = flow do
      button "Shoes Info" do
        infoscreen
      end
      button "Clear Image Cache" do
        cachescreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/
        button "Jail Break Gems" do
          jailscreen
        end
      end
      button "Manage Gems" do
        gemscreen
      end
      if Shoes::RELEASE_TYPE =~ /TIGHT/ || true # for testing.
        button "Install Gempack" do
          gempack_screen
        end
      end
      button "Profile" do
        require 'shoes/profiler'
        Shoes.profile(nil)
      end
      button "Copy Samples" do
        cp_samples_screen
      end
      button "Packager URLs" do
        pack_screen
      end
      if RUBY_PLATFORM =~ /darwin/
        button "cshoes" do
          cshoes_screen
        end
      end
      button "VLC setup" do
        vlc_screen
      end
      button "Manual" do
        Shoes.show_manual
      end
      button "Splash" do
        Shoes.splash
      end
      button "Quit" do
         Shoes.quit
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
  
  def vlc_screen
    require_relative 'vlcpath'
    Vlc_path.load File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
    @panel.clear
    @panel.append do
      para "Set paths for your VLC Installation. Pick the libvlc.dll for \
the first selection and then the Folder named plugins"
      flow do
        para "libvlc.dll [.so, .dylib]"
        @vlcapp = edit_line "#{ENV['VLC_APP_PATH']}", width: 320
        button "Update" do
          vlcp = ask_open_file
          if vlcp
            @vlcapp.text = vlcp
          end
        end
      end
      flow do
        para "VLC Plugin Path "
        @vlcplug = edit_line "#{ENV['VLC_PLUGIN_PATH']}", width: 350
        button "Update" do
          vlpp = ask_open_folder
          if vlpp
            @vlcplug.text = vlpp
          end
       end
     end
      flow do
        button "Save" do
          if ! (Vlc_path.check @vlcapp.text, @vlcplug.text)
            alert "Those are not good paths. Try again."
          else 
            ENV['VLC_APP_PATH'] = @vlcapp.text
            ENV['VLC_PLUGIN_PATH'] = @vlcplug.text
            mkdir_p File.join(LIB_DIR, Shoes::RELEASE_NAME)
            Vlc_path.save File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
          end
        end
        button "Remove" do
          ENV['VLC_APP_PATH'] =  @vlcapp.text = ''
          ENV['VLC_PLUGIN_PATH'] = @vlcplug.text = ''
          rm File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
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
      para "Chose a directory that you want the samples directories"
      para "to be created inside of."
      button "Select Directory for a copy to" do
        # OSX is a bit brain dead for ask_save_folder
        if destdir = ask_save_folder()
          @panel.append do
            para "Copy #{DIR}/samples/* to #{destdir} ?"
            button "OK" do
              @panel.append do
                @lb = edit_box width: 400
              end
              ary = []
              require 'fileutils'
              mkdir_p destdir
              sampdir = File.join DIR, 'samples'
              ary.push "In #{destdir}"
              cd sampdir do
                Dir.glob('*') do |d|   # simple, good, expert in a perfect world
                  mkdir_p "#{destdir}/#{d}"
                  Dir.glob("#{d}/*").each do |f|
                    cp_r f, "#{destdir}/#{d}"
                    ary << f
                  end
                end
                @lb.text = ary.join("\n")
              end
              #alert "copied"
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
      @cflow = flow do
        button "Delete Image Cache" do
          app.cache_clear :all if @cache_all.checked? 
          app.cache_clear :memory if @cache_int.checked? 
          app.cache_clear :external if @cache_ext.checked?
          quit if confirm "You should restart Shoes"
        end
        @cache_all = radio :imgcache; para "Both caches"
        @cache_int = radio :imgcache ; para "Internal images"
        @cache_ext = radio :imgcache; para "External images"
        @cache_all.checked = true
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

  def tar_extract opened_file
    Gem::Package::TarReader.new( Zlib::GzipReader.new(opened_file)) do |tar|
      tar.each do |entry|
        dest = entry.full_name
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
	      alert "Cannot convert Symlinks. Contact #{hdr.creator}"
	    end
      end
    end
  end

  def gem_copy_to_home srcdir, dest
    gems = Dir.glob("#{srcdir}/*/*")
    mkdir_p dest
    #return if !confirm "#{gems} from #{srcdir} to #{dest}"
    gems.each do |gempath|
      # look inside for the gem.build_complete
      gemn = File.split(gempath)[1]
      if File.exists? File.join(gempath,'gem.build_complete')
        extpath = File.join(dest, 'extensions', "#{Gem::Platform.local}", '2.1.0', gemn)
        puts extpath
        mkdir_p extpath
        cp File.join(gempath,'gem.build_complete'), extpath
      end
      # copy the gemspec
      specpath = File.join(dest, 'specifications')
      mkdir_p specpath
      specname = gemn+'.gemspec'
      cp File.join(gempath,'gemspec'), File.join(specpath, specname)
      # copy ext if we have one
      if File.exists? File.join(gempath, 'ext')
        puts "Copy ext #{gempath}"
        mkdir_p File.join(dest, 'gems', gemn)
        cp_r File.join(gempath, 'ext'), File.join(dest,'gems', gemn)
      end
      # copy lib if we have it
      if File.exists? File.join(gempath, 'lib')
        mkdir_p File.join(dest, 'gems', gemn)
        cp_r File.join(gempath, 'lib'), File.join(dest,'gems', gemn)
      end
    end
    gemlist = []
    gems.each {|g| gemlist << File.basename(g) }
    return gemlist
  end

  def gempack_helper tgzpath
    # make a temp directory and unpack the tgzpath into it
    # loop thru the 'special' gems in there and copy into GEM_DIR
    td = Dir.mktmpdir('gempack')
    tarf = File.open(tgzpath,'rb')
    Dir.chdir(td) do |d|
      tar_extract tarf # if confirm "Copy #{tgzpath} to #{GEM_DIR} via #{td}"
    end
    #just begining to get ugly -FIXME -- need th
    return gem_copy_to_home td, GEM_DIR
  end

  def gempack_screen
    @panel.clear
    @panel.append do
      para "Load binary gems from a tgz file. Be Careful! - You can break Shoes \
with the wrong package for your plaftorm!"
      button "Select file..." do
        gempack = ask_open_file
        if gempack
          gemslist = gempack_helper gempack
          gemslist.each do |g|
            para "Installed #{g}\n"
          end
          para "---------------------------\n"
          para "You must quit and restart Shoes to see these gems"
        end
      end
    end
   end

   # not that pretty at all.
   def rewrite_with_env before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
     File.open(after, 'w') do |a|
       File.open(before) do |b|
         b.each do |line|
           a << line.gsub(reg) do
             if reg2.include? '\1'
               reg2.gsub(%r!\\1!, ENV[$1])
             else
               reg2
             end
           end
         end
       end
     end
   end


  def cshoes_screen
    @panel.clear
    @panel.append do
      stack do
          para "Create a cshoes shell script for terminal users to use. \
OSX has to use absolute paths for launching and cshoes script tries to compensate \
but it needs to know where Shoes is"
          para "Select the Shoes.app to point to. "
          this_shoes = "/Applications/Shoes.app"
          flow do
            @shoes_loc = edit_line this_shoes, :width => 300
            button "Select..." do
              this_shoes = ask_open_file
              if this_shoes
                @shoes_loc.text = this_shoes
              end
            end
          end
          new_loc = "#{ENV['HOME']}"
          flow do
            para "Save here:  "
            @cshoes_loc = edit_line new_loc
            button "Select ..." do
              new_loc = ask_save_folder
              if new_loc
                @cshoes_loc.text = new_loc
              end
            end
          end
          flow do
            para "Ready? "
            button "Create Script" do
              if confirm "Create #{@cshoes_loc.text}/cshoes Using #{@shoes_loc.text}"
                #prepare for rewrite - expect complaints about constants
                ENV['SHOES_RUBY_ARCH'] = RUBY_PLATFORM
                ENV['APPNAME'] = 'Shoes'
                ENV['TGT_DIR'] = @shoes_loc.text
                rewrite_with_env  File.join(DIR,"static/stubs/cshoes.templ"), "#{@cshoes_loc.text}/cshoes"
                chmod 0755, "#{@cshoes_loc.text}/cshoes"
              end
            end
          end
        end
      end
    end
end # App
