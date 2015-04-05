# cross compile for win7 on linux
# save the running ruby path+name First
bindir = RbConfig::CONFIG['bindir']
rbname = RbConfig::CONFIG['ruby_install_name']
# remove RbConfig:: constants
RbConfig::send :remove_const, :TOPDIR
RbConfig::send :remove_const, :CONFIG
RbConfig::send :remove_const, :MAKEFILE_CONFIG
# puts "load new consts from #{ENV['EXT_RBCONFIG']}"
load "#{ENV['EXT_RBCONFIG']}"
# restore running ruby path and name
RbConfig::CONFIG['bindir'] = bindir
RbConfig::MAKEFILE_CONFIG['bindir'] = bindir
RbConfig::CONFIG['ruby_install_name'] = rbname
RbConfig::MAKEFILE_CONFIG['ruby_install_name'] = rbname
# Not all of the below lines are needed. Doesn't hurt to make sure.
rblv = ENV['TGT_RUBY_V']
rbroot = ENV['TGT_RUBY_PATH']
rlib = rbroot+"/bin"
incl = "#{rbroot}/include/ruby-#{rblv}"
incla = "#{incl}/#{ENV['TGT_ARCH']}"
# for the 'have' tests
RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC']
RbConfig::CONFIG["rubyhdrdir"] = incl
RbConfig::CONFIG["rubyarchhdrdir"] = incla
RbConfig::CONFIG['libdir'] = rlib 
RbConfig::CONFIG['rubylibdir'] = rlib 
# for building the ext (in the generated Makefile)
RbConfig::MAKEFILE_CONFIG["rubyhdrdir"] = incl
RbConfig::MAKEFILE_CONFIG["rubyarchhdrdir"] = incla
RbConfig::MAKEFILE_CONFIG['libdir'] = rlib 
RbConfig::MAKEFILE_CONFIG['rubylibdir'] = rlib 

require 'mkmf'
CONFIG['CC']=ENV['CC'] if ENV['CC']
# Add flags to silence compiler warnings.
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math -Wno-unused-variable -Wno-return-type'
$LDFLAGS = "-L #{rbroot}/bin"
$LIBS = ""
CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']
 
create_makefile('chipmunk')
