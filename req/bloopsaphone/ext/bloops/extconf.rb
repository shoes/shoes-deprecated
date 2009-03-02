require 'mkmf'
require 'fileutils'

$CFLAGS << " -I../../c "

have_library("portaudio")
create_makefile("bloops")
