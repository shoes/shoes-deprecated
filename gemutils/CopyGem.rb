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
       # HACK ALERT - HARD CODED
       inRuby = ['bigdecimal', 'io-console', 'json', 'psych' , 'rake', 'test-unit']
       inRuby.each do |key| 
         @deplist.delete key
       end
       @deplist.each do |nm, dep|
         para "#{nm} => #{dep.full_name} #{Gem::Platform.local}"
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
    alert "empty destination" if @dirfld.text == ""
    srcpath = @srcfld.text
    destpath =  @dirfld.text
    if RUBY_PLATFORM =~ /mingw/
      srcpath.gsub!(/\\/, '/')  
      destpath.gsub!(/\\/, '/')  
    end

    @deplist.each do |name, spec|
      puts "copy #{spec.full_name} to #{destpath}"
      mkdir_p(File.join(destpath, spec.full_name))
      cp File.join(srcpath, 'specifications', "#{spec.full_name}.gemspec") , 
         File.join(destpath, spec.full_name,'gemspec')
      # check for binary code
      rubyv = RUBY_VERSION[/\d.\d/]+'.0'
      gemcompl = File.join(srcpath, 'extensions', "#{Gem::Platform.local}",
         rubyv, spec.full_name, 'gem.build_complete')
      if File.exist? gemcompl
        puts "binary #{gemcompl}"
        cp gemcompl, File.join(destpath, spec.full_name)
      end 
      # copy lib/ or ext/ or whatever the spec says.
      # caution: spec is modified by the eval() so it's filled in with stuff
      puts "Require paths #{spec.require_paths}"
      skip1 = spec.require_paths
      if (skip1.length > 1) &&  (skip1.include? 'ext')
       # A confused gem! Me too. Perhaps Any binary gem?
       src = File.join(skip1)
       dest = File.join(destpath, spec.full_name, skip1[1])
       mkdir_p dest
       puts "weird ext copy #{src} -> #{dest}"
       cp_r src, dest
      elsif (skip1.length > 1) &&  (skip1.include? 'lib')
        puts "weird lib copy  #{}"
        cp_r File.join(srcpath,'gems', spec.full_name, 'lib'), File.join(destpath, spec.full_name)
      else
        skip1.each do |rqp| 
          puts "copy this  #{rqp}"
          cp_r File.join(srcpath,'gems', spec.full_name, rqp), File.join(destpath, spec.full_name)
        end
      end
    end 
    if @tgzfld.checked? 
      create_tar if confirm "Tar up #{destpath}"
    end
  end
  
  def create_tar
    puts "create_tar called"
  end
  
end
