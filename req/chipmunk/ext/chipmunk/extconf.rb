require 'mkmf'

if ARGV[0]  == "macosx"
    $CFLAGS += ' -arch ppc -arch i386 -arch x86_64'
    $LDFLAGS += ' -arch x86_64 -arch i386 -arch ppc'
end

$CFLAGS += ' -std=gnu99 -ffast-math'
create_makefile('chipmunk')
