
require 'mkmf'
# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. There might be a better way.
# OK. With the mingw stuff, it's a hack. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-shorten-64-to-32 -Wno-return-type -Wno-unused-variable'
$CFLAGS += ' -Wno-implicit-function-declaration -Wno-unused-function'

create_makefile('chipmunk')
