incdir = ""
ARGV.each do |dir| 
  incdir << "-I#{dir} "
  puts incdir
end
workrb = `which ruby`.chomp
require 'mkmf'
$ruby = workrb
dir_config("hpricot_scan")
have_library("c", "main")
create_makefile("hpricot_scan")

