# This is for a native build (loose shoes)
# It is safe and desireable to use RbConfig::CONFIG settings
#   Will not build gems and most extentions
#   Links against system (or rvm) ruby, and libraries. No LD_LIB_PATH
require 'rbconfig'

# manually set below to what you want to build with/for
#ENV['DEBUG'] = "true" # turns on the call log
APP['GTK'] = "gtk+-2.0"
#APP['GTK'] = "gtk+-3.0"
ENV['GDB'] = "true" # compile -g,  don't strip symbols
if ENV['GDB']
  LINUX_CFLAGS = "-g -O0"
else
  LINUX_CFLAGS = "-O -Wall"
end
# figure out which ruby we need.
rv =  RUBY_VERSION[/\d.\d/]

LINUX_CFLAGS << " -DRUBY_HTTP"
LINUX_CFLAGS << " -DRUBY_1_9"
LINUX_CFLAGS << " -DDEBUG" if ENV['DEBUG']
LINUX_CFLAGS << " -DGTK3" unless APP['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -shared"
# Following line may need handcrafting
LINUX_CFLAGS << " -I/usr/include/"
LINUX_CFLAGS << " #{`pkg-config --cflags #{APP['GTK']}`.strip}"

CC = "gcc"
if APP['GTK'] == 'gtk+-2.0'
  file_list = %w(shoes/native/gtk.c shoes/http/rbload.c) + ["shoes/*.c"] + ["shoes/console/*.c"]
else
  file_list = %w(shoes/native/gtk.c shoes/native/gtkfixedalt.c shoes/native/gtkentryalt.c
               shoes/native/gtkcomboboxtextalt.c shoes/native/gtkbuttonalt.c
               shoes/native/gtkscrolledwindowalt.c shoes/native/gtkprogressbaralt.c 
               shoes/http/rbload.c) + ["shoes/*.c"] + ["shoes/console/*.c"]
end
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

# Query pkg-config for cflags and link settings
EXT_RUBY = RbConfig::CONFIG['prefix']
RUBY_CFLAGS = " #{`pkg-config --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip}"
# Ruby 2.1.2 with RVM has a bug. Workaround or wait for perfection?
rlib = `pkg-config --libs #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip
if rlib[/{ORIGIN/]
  #abort "Bug found #{rlib}"
  RUBY_LIB = rlib.gsub(/\$\\{ORIGIN\\}/, "#{EXT_RUBY}/lib")
else
  RUBY_LIB = rlib
end
CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip
GTK_FLAGS = "#{`pkg-config --cflags #{APP['GTK']}`.strip}"
GTK_LIB = "#{`pkg-config --libs #{APP['GTK']}`.strip}"

MISC_LIB = " -lgif -ljpeg "

# collect flags together
LINUX_CFLAGS << " #{RUBY_CFLAGS} #{GTK_FLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"

# collect link settings together. Does order matter?
LINUX_LIBS = "#{RUBY_LIB} #{GTK_LIB}  #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"
LINUX_LIBS << " -lfontconfig" if APP['GTK'] == "gtk+-3.0"
# the following is only used to link the shoes code with main.o
LINUX_LDFLAGS = "-L. -rdynamic -Wl,-export-dynamic"

# somebody needs the below Constants
ADD_DLL = []
DLEXT = "so"
