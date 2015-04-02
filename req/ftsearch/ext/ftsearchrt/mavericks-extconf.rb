# mavericks - clang loves to complain
require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-shorten-64-to-32 -Wno-return-type -Wno-unused-variable'
$CFLAGS += ' -Wno-implicit-function-declaration -Wno-unused-function'

create_makefile('ftsearchrt')

