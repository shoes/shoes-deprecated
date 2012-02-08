incdir = ""
ARGV.each do |dir| 
  incdir << "-I#{dir} "
  puts incdir
end
workrb = `which ruby`.chomp
require 'mkmf'
$ruby = workrb
$CFLAGS += ' -std=gnu99 -ffast-math'
create_makefile('chipmunk')
