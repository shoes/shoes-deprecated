# This is for a native build (loose shoes)
# It is safe and desireable to use RbConfig::CONFIG settings
#   Will not build gems and most extentions 
#   Links against system (or rvm) ruby, and libraries. No LD_LIB_PATH
require 'rbconfig'

# manually set below to what you want to build with/for
#ENV['DEBUG'] = "true" # turns on the call log in shoes/gtk
#ENV['GTK'] = "gtk+-3.0" # pick this or the next
ENV['GTK'] = "gtk+-2.0"
ENV['GDB'] = "true" # compile -g,  don't strip symbols
# Use curl or Ruby for http downloads.
RUBY_HTTP = true
# Pick your optimatization and debugging options
if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = "-g -O0"
else
  LINUX_CFLAGS = "-O -Wall"
end
# figure out which ruby we need.
rv =  RUBY_VERSION[/\d.\d/]
#puts "Ruby V: #{rv}"
# Add the -Defines for shoes code
#  use Ruby for HTTP
LINUX_CFLAGS << " -DRUBY_HTTP" if RUBY_HTTP
LINUX_CFLAGS << " -DRUBY_1_9"
LINUX_CFLAGS << " -DGTK3" unless ENV['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -shared"
# Following line may need handcrafting 
LINUX_CFLAGS << " -I/usr/include/"
LINUX_CFLAGS << " #{`pkg-config --cflags #{ENV['GTK']}`.strip}"

CC = "gcc"
if RUBY_HTTP
  file_list = %w{shoes/native/gtk.c shoes/http/rbload.c} + ["shoes/*.c"]
else
  file_list = %w{shoes/native/gtk.c shoes/http/curl.c} + ["shoes/*.c"]
end
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

# Query pkg-config for cflags and link settings
EXT_RUBY = RbConfig::CONFIG['prefix']
RUBY_CFLAGS = " #{`pkg-config --cflags #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip}"
RUBY_LIB = `pkg-config --libs #{EXT_RUBY}/lib/pkgconfig/ruby-#{rv}.pc`.strip
CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip
GTK_FLAGS = "#{`pkg-config --cflags #{ENV['GTK']}`.strip}"
GTK_LIB = "#{`pkg-config --libs #{ENV['GTK']}`.strip}"
CURL_LIB = `curl-config --libs`.strip
#MISC_LIB = " -lungif -ljpeg "
MISC_LIB = " -lgif -ljpeg "

# collect flags together
LINUX_CFLAGS << " #{RUBY_CFLAGS} #{GTK_FLAGS} #{CAIRO_CFLAGS} #{PANGO_CFLAGS}"

# collect link settings together. Does order matter? 
LINUX_LIBS = "#{RUBY_LIB} #{GTK_LIB} #{CURL_LIB if !RUBY_HTTP} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"
# the following is only used to link the shoes code with main.o
LINUX_LDFLAGS = "-L. -rdynamic -Wl,-export-dynamic"

# somebody needs the below Constants
ADD_DLL = []
DLEXT = "so"
