
require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-function -Wno-shorten-64-to-32 -Wno-format'
$CFLAGS += ' -Wno-uninitialized -Wno-unused-const-variable'

dir_config("hpricot_scan")
#have_library("c", "main")
create_makefile("hpricot_scan")
