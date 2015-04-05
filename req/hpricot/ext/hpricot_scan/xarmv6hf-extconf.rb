
rblv = ENV['TGT_RUBY_V']
rbroot = ENV['TGT_RUBY_PATH']
rlib = rbroot+"/lib"
incl = "#{rbroot}/include/ruby-#{rblv}"
incla = "#{incl}/#{ENV['TGT_ARCH']}"
RbConfig::CONFIG["rubyhdrdir"] = incl
RbConfig::CONFIG["rubyarchhdrdir"] = incla
RbConfig::MAKEFILE_CONFIG['libdir'] = rlib # needed for Linking ext.so
RbConfig::CONFIG['libdir'] = rlib # needed for conftest

require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-function -Wno-type-limits -Wno-extra'

dir_config("hpricot_scan")
#have_library("c", "main")
create_makefile("hpricot_scan")
