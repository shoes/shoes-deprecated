require 'mkmf'

$CFLAGS << " -Iincludes "
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
$CFLAGS << "-DSHOES_MINGW32 " if RUBY_PLATFORM =~ /mingw/
have_library("z")
create_makefile("binject")
