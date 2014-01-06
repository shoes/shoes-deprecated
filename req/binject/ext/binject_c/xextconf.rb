workrb = `which ruby`.chomp
require 'mkmf'
$ruby = workrb
$CFLAGS << " -Iincludes "
$CFLAGS << "-I#{ARGV[0]}/include " if ARGV.length > 0
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
$CFLAGS << "-DSHOES_MINGW32 " if CONFIG['arch'] =~ /mingw/
have_library("z")
create_makefile("binject")
