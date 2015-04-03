
require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-function -Wno-uninitialized -Wno-extra'

dir_config("hpricot_scan")
#have_library("c", "main")
create_makefile("hpricot_scan")
