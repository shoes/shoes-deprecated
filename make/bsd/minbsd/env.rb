# This is for a freebds only build (loose shoes)
# It is safe and desireable to use RbConfig::CONFIG settings
#   Will not build gems or copy gems - uses the host ruby.
#   Cannot be distributed. 
require 'rbconfig'

APP['GDB'] = "true" # true => compile -g,  don't strip symbols
if APP['GDB']
  LINUX_CFLAGS = "-g -O0"
else
  LINUX_CFLAGS = "-O -Wall"
end

# figure out which ruby we need.
rv =  RUBY_VERSION[/\d.\d/]

LINUX_CFLAGS << " -DRUBY_HTTP -DBSD"
LINUX_CFLAGS << " -DRUBY_1_9"
LINUX_CFLAGS << " -DDEBUG" if ENV['DEBUG']
LINUX_CFLAGS << " -DSHOES_GTK -fPIC"
# Following line may need handcrafting
LINUX_CFLAGS << " -I/usr/include/"
LINUX_CFLAGS << " #{`pkgconf --cflags gtk+-3.0`.strip}"

CC = "cc"

# Query pkg-config for cflags and link settings
EXT_RUBY = RbConfig::CONFIG['prefix']
RUBY_CFLAGS = " #{`pkgconf --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip}"
# Ruby 2.1.2 with RVM has a bug. Workaround or wait for perfection?
rlib = `pkgconf --libs #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip
# 2.2.3 is missing  -L'$${ORIGIN}/../lib' in LIBRUBYARG_SHARED in .pc
if !rlib[/\-L/]
  #puts "missing -L in #{rlib}" 
  rlib = "-L#{EXT_RUBY}/lib "+rlib
end
if rlib[/{ORIGIN/]
  #abort "Bug found #{rlib}"
  RUBY_LIB = rlib.gsub(/\$\\{ORIGIN\\}/, "#{EXT_RUBY}/lib")
  #RUBY_LIB = rlib
else
  RUBY_LIB = rlib
end
CAIRO_CFLAGS = `pkgconf --cflags cairo`.strip
CAIRO_LIB = `pkgconf --libs cairo`.strip
PANGO_CFLAGS = `pkgconf --cflags pango`.strip
PANGO_LIB = `pkgconf --libs pango`.strip
GTK_FLAGS = "#{`pkgconf --cflags gtk+-3.0`.strip}"
GTK_LIB = "#{`pkgconf --libs gtk+-3.0`.strip}"

MISC_LIB = " -lgif -ljpeg"

# don't use pkg-config for librsvg-2.0 - a warning.
MISC_CFLAGS = ' '

MISC_CFLAGS << "-I/usr/local/include/librsvg-2.0/librsvg "
MISC_LIB << " /usr/local/lib/librsvg-2.so"

# collect flags together
LINUX_CFLAGS << " #{RUBY_CFLAGS} #{GTK_FLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS} #{MISC_CFLAGS}"

# collect link settings together. Does order matter?
LINUX_LIBS = "#{RUBY_LIB} #{GTK_LIB}  #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"
LINUX_LIBS << " -lfontconfig" # if APP['GTK'] == "gtk+-3.0"
# the following is only used to link the shoes code with main.o
LINUX_LDFLAGS = "-L. -rdynamic -Wl,-export-dynamic"

# Main Rakefile and tasks.rb needs the below Constants
ADD_DLL = []
DLEXT = "so"
SOLOCS = {} # needed to match Rakefile expectations.
=begin
# to save settings 
bld_args = {}
bld_args['CC'] = CC
bld_args['ADD_DLL'] = []
bld_args['DLEXT'] = "so"
bld_args['SOLOCS'] = {}
bld_args['LINUX_CFLAGS'] = LINUX_CFLAGS
bld_args['LINUX_LDFLAGS'] = LINUX_LDFLAGS
bld_args['LINUX_LIBS'] = LINUX_LIBS
File.open("#{TGT_DIR}/build.yaml", 'w') {|f| YAML.dump(bld_args, f)}
=end
