
require 'mkmf'

$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-variable -Wno-implicit-function-declaration -Wno-unused-function'
$CFLAGS += ' -Wno-implicit-function-declaration -Wno-return-type -Wno-pointer-to-int-cast'
$CFLAGS += ' -Wno-format'
create_makefile('chipmunk')
