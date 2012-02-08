incdir = ""
ARGV.each do |dir| 
  incdir << "-I#{dir} "
  puts incdir
end
workrb = `which ruby`.chomp
require 'mkmf'
$ruby = workrb
create_makefile 'parser'
