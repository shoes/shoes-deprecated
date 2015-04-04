
require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-function'

#have_header('stdio.h') or exit
dir_config('fast_xs')
create_makefile('fast_xs')
#abort "in fast_xs CC = #{CONFIG['CC']}"
