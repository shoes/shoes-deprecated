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
# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
#$CFLAGS += ' -Wno-unused-function -Wno-unused-const-variable -Wno-shorten-64-to-32'
$CFLAGS += ' -Wno-unused-function '
$LDFLAGS = "-L #{rbroot}/bin"
#puts "$LIBS = #{$LIBS}"
$LIBS = ""
CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']

#have_header('stdio.h') or exit
dir_config('fast_xs')
create_makefile('fast_xs')
#abort "in fast_xs CC = #{CONFIG['CC']}"
