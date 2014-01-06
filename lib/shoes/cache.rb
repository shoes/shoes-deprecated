require 'fileutils'
include FileUtils

# locate ~/.shoes
require 'tmpdir'
lib_dir = nil
homes = []
homes << [ENV['HOME'], File.join( ENV['HOME'], '.shoes' )] if ENV['HOME']
homes << [ENV['APPDATA'], File.join( ENV['APPDATA'], 'Shoes' )] if ENV['APPDATA']
homes.each do |home_top, home_dir|
  next unless home_top
  if File.exists? home_top
    lib_dir = home_dir
    break
  end
end
LIB_DIR = lib_dir || File.join(Dir::tmpdir, "shoes")
LIB_DIR.gsub! /\\/, '/'

#' Stupid comment quote for Geany syntax color matching bug
tight_shoes = Shoes::RELEASE_TYPE =~ /TIGHT/

if tight_shoes
  require 'rbconfig'
  SITE_LIB_DIR = File.join(LIB_DIR, '+lib')
  GEM_DIR = File.join(LIB_DIR, '+gem')
  $:.unshift SITE_LIB_DIR
  $:.unshift GEM_DIR
  ENV['GEM_HOME'] = GEM_DIR
else
  #puts "LOOSE Shoes #{RUBY_VERSION}"
  $:.unshift ENV['GEM_HOME'] if ENV['GEM_HOME']
  # FIXME add path for shoes extensions don't hardcode 1.9.1
  $: << DIR+"/lib/ruby/1.9.1"
  $: << DIR+"/lib/ruby/1.9.1/#{RbConfig::CONFIG['arch']}"
  $: << DIR+"/lib/shoes"
  # May encounter ENV['GEM_PATH'] in the wild.
  #if ENV['GEM_PATH']
  #  ENV['GEM_PATH'].split(':').each {|p| $:.unshift p }
  #end
end

CACHE_DIR = File.join(LIB_DIR, '+cache')
mkdir_p(CACHE_DIR)
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']

if tight_shoes 
  config = {
	  'ruby_install_name' => "shoes --ruby",
	  'RUBY_INSTALL_NAME' => "shoes --ruby",
	  'prefix' => "#{DIR}", 
	  'bindir' => "#{DIR}", 
	  'rubylibdir' => "#{DIR}/ruby/lib",
	  'datarootdir' => "#{DIR}/share",
	  'dvidir' => "#{DIR}/doc/${PACKAGE}",
	  'psdir' => "#{DIR}/doc/${PACKAGE}",
	  'htmldir' => "#{DIR}/doc/${PACKAGE}",
	  'docdir' => "#{DIR}/doc/${PACKAGE}",
	  'archdir' => "#{DIR}/ruby/lib/#{SHOES_RUBY_ARCH}",
	  'sitedir' => SITE_LIB_DIR,
	  'sitelibdir' => SITE_LIB_DIR,
	  'sitearchdir' => "#{SITE_LIB_DIR}/#{SHOES_RUBY_ARCH}",
	  'LIBRUBYARG_STATIC' => "",
	  'libdir' => "#{DIR}",
	  'LDFLAGS' => "-L. -L#{DIR}",
	  'rubylibprefix' => "#{DIR}/ruby"
  }
  RbConfig::CONFIG.merge! config
  RbConfig::MAKEFILE_CONFIG.merge! config
  # Add refs to Shoes builtin Gems (but not exts?)
  GEM_CENTRAL_DIR = File.join(DIR, 'ruby/gems/' + RbConfig::CONFIG['ruby_version'])
  Dir[GEM_CENTRAL_DIR + "/gems/*"].each do |gdir|
    $: << "#{gdir}/lib"
  end
  puts "Jailbreak location #{Dir.pwd}"
  # Jailbreak for Gems. Load then a from a pre-existing ruby's gems
  if File.exist? 'getoutofjail.card'
    if ENV['GEM_PATH']
      ENV['GEM_PATH'].split(':').each {|p| $:.unshift p }
    end
    require 'rubygems'
    ShoesGemJailBreak = true
  else
    ShoesGemJailBreak = false
  end
else
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
