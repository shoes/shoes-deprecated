require 'fileutils'
include FileUtils
# add it download patches
require_relative 'download.rb'
# locate ~/.shoes
require 'tmpdir'
lib_dir = nil
homes = []
homes << [ENV['LOCALAPPDATA'], File.join( ENV['LOCALAPPDATA'], 'Shoes')] if ENV['LOCALAPPDATA']
homes << [ENV['APPDATA'], File.join( ENV['APPDATA'], 'Shoes' )] if ENV['APPDATA']
homes << [ENV['HOME'], File.join( ENV['HOME'], '.shoes' )] if ENV['HOME']
homes.each do |home_top, home_dir|
  next unless home_top
  if File.exists? home_top
    lib_dir = home_dir
    break
  end
end
LIB_DIR = lib_dir || File.join(Dir::tmpdir, "shoes")
#LIB_DIR.gsub! /\\/, '\/'
LIB_DIR.gsub! /\\+/, "/"

tight_shoes = Shoes::RELEASE_TYPE =~ /TIGHT/
rbv = RbConfig::CONFIG['ruby_version']
if tight_shoes 
  require 'rbconfig'
  SITE_LIB_DIR = File.join(LIB_DIR, '+lib')
  GEM_DIR = File.join(LIB_DIR, '+gem')
  $:.unshift SITE_LIB_DIR
  $:.unshift GEM_DIR
  ENV['GEM_HOME'] = GEM_DIR
else
  #puts "LOOSE Shoes #{RUBY_VERSION} #{DIR}"
  $:.unshift ENV['GEM_HOME'] if ENV['GEM_HOME']
  rv = case RUBY_VERSION
    when /1.9/
      '1.9.1'
    when /2.0.0/
      '2.0.0'
    when /2.1/
      '2.1.0'
    when /2.2/
      '2.2.0'
    when /2.3/
      '2.3.0'
    else
      RUBY_VERSION
  end
  $:.unshift DIR+"/lib/ruby/#{rv}/#{RbConfig::CONFIG['arch']}"
  $:.unshift DIR+"/lib/ruby/#{rv}"
  $:.unshift DIR+"/lib/shoes"
  # May encounter ENV['GEM_PATH'] in the wild.
  #if ENV['GEM_PATH']
  #  ENV['GEM_PATH'].split(':').each {|p| $:.unshift p }
  #end
end

CACHE_DIR = File.join(LIB_DIR, '+cache')
mkdir_p(CACHE_DIR)
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']

if tight_shoes 
  #puts "Dir: #{DIR} #{RbConfig::CONFIG["oldincludedir"]}"
  incld = "#{DIR}/lib/ruby/include/ruby-1.9.1"
  config = {
	  'ruby_install_name' => "shoes --ruby",
	  'RUBY_INSTALL_NAME' => "shoes --ruby",
	  'prefix' => "#{DIR}", 
	  'bindir' => "#{DIR}", 
	  'rubylibdir' => "#{DIR}/lib/ruby",
	  'includedir' => incld,
	  'rubyhdrdir' => incld,
	  'vendorhdrdir' => incld,
	  'sitehdrdir' => incld,
	  'datarootdir' => "#{DIR}/share",
	  'dvidir' => "#{DIR}/doc/${PACKAGE}",
	  'psdir' => "#{DIR}/doc/${PACKAGE}",
	  'htmldir' => "#{DIR}/doc/${PACKAGE}",
	  'docdir' => "#{DIR}/doc/${PACKAGE}",
#	  'archdir' => "#{DIR}/ruby/lib/#{SHOES_RUBY_ARCH}",
	  'archdir' => "#{DIR}/lib/ruby/1.9.1/#{SHOES_RUBY_ARCH}",
	  'sitedir' => SITE_LIB_DIR,
	  'sitelibdir' => SITE_LIB_DIR,
	  'sitearchdir' => "#{SITE_LIB_DIR}/#{SHOES_RUBY_ARCH}",
	  'LIBRUBYARG_STATIC' => "",
	  'libdir' => "#{DIR}",
	  'LDFLAGS' => "-L. -L#{DIR}",
	  'rubylibprefix' => "#{DIR}/ruby"
  }
  #debug "DYLD = #{ENV['DYLD_LIBRARY_PATH']} DIR = #{DIR}"
  RbConfig::CONFIG.merge! config
  RbConfig::MAKEFILE_CONFIG.merge! config
  # Add refs to Shoes builtin Gems (but not exts?)
  GEM_CENTRAL_DIR = File.join(DIR, 'lib/ruby/gems/' + RbConfig::CONFIG['ruby_version'])
  Dir[GEM_CENTRAL_DIR + "/gems/*"].each do |gdir|
    $: << "#{gdir}/lib"
  end
  #jloc = "#{ENV['HOME']}/.shoes/#{Shoes::RELEASE_NAME}/getoutofjail.card"
  jloc = File.join(LIB_DIR, Shoes::RELEASE_NAME, 'getoutofjail.card')
  #puts "Jailbreak location #{jloc}"
  # Jailbreak for Gems. Load them a from a pre-existing ruby's gems
  # or file contents
  if File.exist? jloc
    open(jloc, 'r') do |f|
      f.each do |l| 
        ln = l.strip
        $:.unshift ln if ln.length > 0
      end
    end
    if ENV['GEM_PATH']
      ENV['GEM_PATH'].split(':').each {|p| $:.unshift p }
    end
    require 'rubygems'
    ShoesGemJailBreak = true
  else
    if ENV['GEM_PATH']
      # disable GEM_PATH if its in use otherwise funny things occur.
      ENV.delete('GEM_PATH')
    end
    ShoesGemJailBreak = false
  end
else # Loose Shoes
  ShoesGemJailBreak = true
  # 'rubylibprefix' then 'libdir' for gem's rb and so
  # FIXME -  lib/shoes/setup.rb uses GEM_DIR and GEM_CENTRAL_DIR 
  # Set this to where the users/system Ruby keeps things.
  if ENV['GEM_HOME'] && ENV['GEM_HOME'] =~ /home\/.*\/.rvm/
	GEM_CENTRAL_DIR = GEM_DIR =  ENV['GEM_HOME']
  else
    puts "Please set GEM_HOME env var or use rvm"
  end
end
# find vlc libs
require_relative 'vlcpath'
yamlp = File.join(LIB_DIR, Shoes::RELEASE_NAME, 'vlc.yaml')
Vlc_path.load yamlp

