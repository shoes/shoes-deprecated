require 'mkmf'

$CFLAGS << " -Iincludes "
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
have_library("z")
create_makefile("binject")
