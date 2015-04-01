require 'mkmf'

$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-variable -Wno-implicit-function-declaration -Wno-unused-function'
$CFLAGS += ' -Wno-implicit-function-declaration -Wno-return-type -Wno-pointer-to-int-cast'
$CFLAGS += ' -Wno-format'
#have_header('stdio.h') or exit
dir_config('fast_xs')
create_makefile('fast_xs')
#abort "in fast_xs CC = #{CONFIG['CC']}"
