workrb = `which ruby`.chomp
require 'mkmf'
$ruby = workrb
$CFLAGS << " -Iincludes "
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
$CFLAGS << "-I#{ARGV[0]}/include " if ARGV.length > 0
$CFLAGS << "-DSHOES_MINGW32 " if CONFIG['arch'] =~ /mingw/
$LDFLAGS << "-L#{ARGV[0]}/lib " if ARGV.length > 0
have_library("z")
create_makefile("binject")
