require 'mkmf'

$CFLAGS << " -Iincludes "

have_library("z")
create_makefile("binject")
