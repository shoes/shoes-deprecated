# copy a gem (and all dependent gems to a a specified location
# for Shoes purposes (only lib, spec, and ext..../gem.build_complete
require 'rubygems/dependency_installer'
# Don't use gems ?
Shoes.app do
  stack do
    flow do
      para "Load gems from:"
      @srcfld = edit_line ENV['GEM_HOME']
      button "select ..." do
        @srcfld.text = ask_open_folder
        Gem.use_paths(@srcfld.text, [GEM_DIR, GEM_CENTRAL_DIR])
        Gem.refresh
      end
      para "Dir with specifications/, gems/, extensions/"
    end
    flow do 
      para "Copy gems to here: "
      @dirfld = edit_line
      button "select..." do
        @dirfld.text = ask_open_folder 
      end
    end
    flow do
      button "List" do
        @panel.clear 
        @panel.append do
          @gemlist = stack
        end
        gem_refresh_local
      end
      @tgzfld = check; para "tgz"
      @cpbtn = button "Copy" do
        copy_gem_files
      end
      button "Quit" do
        exit
      end
    end
    @msglog = edit_box width: 500, height: 200

    @panel = stack do
        @gemlist = stack {}
    end
  end
  
  def gem_refresh_local
    @gemlist.clear
    @gemlist.background white
    @cpbtn.state ='disabled'
    rbpath = @srcfld.text+'/specifications/*.gemspec'
    if RUBY_PLATFORM =~ /mingw/
      rbpath.gsub!(/\\/, '/')  # Dir.glob is special here
    end
    Dir.glob(rbpath).sort.each do |path|
      fnm = File.basename(path)
      fnm.gsub!('.gemspec','')
      @gemlist.append do
        flow margin: 5 do
          check do
            spec = eval(File.read(path))
            show_deps spec
            @cpbtn.state = nil
          end
          para "#{fnm}"
        end
      end
    end
   end
   
   def show_deps spec
     @panel.clear
     @panel.append do
       # get a hash of ALL the dependent gems - order does not matter
       # they've been installed already and we only need to copy. 
       @deplist = {}
       @deplist[spec.name] = spec
       gem_build_deps spec
       # now filter out the built in gems in default (minitest, rake, rdoc)
       # TODO - HARD CODED list
       inRuby = ['bigdecimal', 'io-console', 'json', 'psych' , 'rake', 'test-unit', 'minitest']
       inRuby.each do |key| 
         @deplist.delete key
       end
       @msglog.append "These will be copied\n"
       @deplist.each do |nm, dep|
         #para "#{nm} => #{dep.full_name} #{Gem::Platform.local}"
         @msglog.append "#{nm} => #{dep.full_name}\n"
       end
     end
   end
   
   def gem_build_deps spec
     #puts "spec #{spec.name}"
     return if spec == nil
     speclist = spec.dependent_specs # returns spec [] or element
     if speclist.instance_of? Array
       speclist.each do |spec| 
         @deplist[spec.name] = spec
         gem_build_deps spec
       end
     end
   end
  
  
  def copy_gem_files
    if @dirfld.text == ""
      alert "empty destination"
      return
    end
    srcpath = @srcfld.text
    destpath =  @dirfld.text
    if RUBY_PLATFORM =~ /mingw/
      srcpath.gsub!(/\\/, '/')  
      destpath.gsub!(/\\/, '/')  
    end
    @msglog.text = ""
    @deplist.each do |name, spec|
      # deal with spec version numbers that are different  - it happens
      # can be multiples - use the last one
      gnm = spec.full_name.gsub(/-\d+.\d+.\d+$/,'')
      lst = Dir.glob("#{srcpath}/specifications/#{gnm}*.gemspec").sort
      #puts "specs #{lst}"
      spec_path = lst[-1]
      # now break it apart 
      use_spec_name = File.basename(spec_path, '.gemspec')
      use_dir = File.dirname(spec_path)
      @msglog.append "copy #{use_spec_name} to #{destpath}\n"
      mkdir_p(File.join(destpath, use_spec_name))
      # 
      cp spec_path, File.join(destpath, use_spec_name, 'gemspec')
      
      # check for binary code
      rubyv = RUBY_VERSION[/\d.\d/]+'.0'
      gemcompl = File.join(srcpath, 'extensions', "#{Gem::Platform.local}",
         rubyv, use_spec_name, 'gem.build_complete')
      #puts "bin check: #{gemcompl}"
      if File.exist? gemcompl
        @msglog.append "binary #{gemcompl}\n"
        cp gemcompl, File.join(destpath, use_spec_name,'gem.build_complete')
      end 
      # copy lib/ or ext/ or whatever the spec says.
      # caution: spec is modified by the eval() so it's filled in with stuff
      @msglog.append "Require paths #{spec.require_paths}\n"
      skip1 = spec.require_paths
      if (skip1.length > 1) &&  (skip1.include? 'ext')
        # A confused gem! Me too. Perhaps Any binary gem?
        src = File.join(skip1)
        dest = File.join(destpath, use_spec_name, skip1[1])
        mkdir_p dest
        @msglog.append "weird ext copy #{src} -> #{dest}\n"
        cp_r src, dest
      elsif (skip1.length > 1) &&  (skip1.include? 'lib')
        @msglog.append "weird lib copy  #{}\n"
        cp_r File.join(srcpath,'gems', use_spec_name, 'lib'), File.join(destpath, use_spec_name)
      else
        skip1.each do |rqp| 
          @msglog.append "copy this  #{rqp}\n"
          cp_r File.join(srcpath,'gems', use_spec_name, rqp), File.join(destpath, use_spec_name)
        end
      end
    end 
    if @tgzfld.checked? 
      create_tar if confirm "Tar up #{destpath}"
    end
  end
  
  def create_tar
    # TODO: create a full gempack here - see coobler.rb for what that is
    puts "create_tar called"
  end
  
end
