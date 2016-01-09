#
# Build shoes for Raspberry pi.  Ruby is built in the chroot/qemu
#
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  ENV['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  APP['GTK'] = custmz['Gtk'] if custmz['Gtk']
else
  # define where your deps are
  ShoesDeps = "/srv/chroot/debrpi"
  EXT_RUBY = "/srv/chroot/debrpi/usr/local"  
  APP['GTK'] = "gtk+-2.0"
end
#ENV['DEBUG'] = "true" # turns on the tracing log
#ENV['GDB'] = nil # compile -g,  strip symbols when nil
CHROOT = ShoesDeps
SHOES_TGT_ARCH = "armv7l-linux-eabihf"
SHOES_GEM_ARCH = "armv7l-linux"
# Specify where the Target system binaries live. 
# Trailing slash is important.
TGT_SYS_DIR = "#{CHROOT}/"
# Setup some shortcuts for the library locations. 
# These are not ruby paths but library paths
arch = 'arm-linux-gnueabihf'
uldir = "#{TGT_SYS_DIR}usr/lib"
ularch = "#{TGT_SYS_DIR}usr/lib/#{arch}"
larch = "#{TGT_SYS_DIR}lib/#{arch}"
# Set appropriately (in my PATH), or use abs path)
CC = "arm-linux-gnueabihf-gcc"
# These ENV vars are used by the extconf.rb files (and tasks.rb)
ENV['SYSROOT'] = CHROOT
ENV['CC'] = CC
ENV['TGT_RUBY_PATH'] = EXT_RUBY
ENV['TGT_ARCH'] = SHOES_TGT_ARCH
ENV['TGT_RUBY_V'] = '2.1.0'
TGT_RUBY_V = ENV['TGT_RUBY_V'] 

pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.1.pc"
pkggtk ="#{ularch}/pkgconfig/#{APP['GTK']}.pc" 

ENV['PKG_CONFIG_PATH'] = "#{ularch}/pkgconfig"
if APP['GTK']== 'gtk+-2.0'
  file_list = ["shoes/console/*.c"] + ["shoes/native/gtk.c"] + ["shoes/http/rbload.c"] + ["shoes/*.c"]
else
  file_list = ["shoes/console/*.c"] + ["shoes/native/*.c"] + ["shoes/http/rbload.c"] + ["shoes/*.c"]
end 
SRC = FileList[*file_list]
OBJ = SRC.map do |x|
  x.gsub(/\.\w+$/, '.o')
end

ADD_DLL = []

# Hand code for your situation 
def xfixip(path)
   path.gsub!(/-I\/usr\//, "-I#{TGT_SYS_DIR}usr/")
   return path
end

def xfixrvmp(path)
  #puts "path  in: #{path}"
  path.gsub!(/-I\/home\/ccoupe\/\.rvm/, "-I/home/cross/armv6-pi/rvm")
  path.gsub!(/-I\/usr\/local\//, "-I#{TGT_SYS_DIR}usr/local/")
  #puts "path out: #{path}"
  return path
end

# Target environment
CAIRO_CFLAGS = `pkg-config --cflags "#{ularch}/pkgconfig/cairo.pc"`.strip
CAIRO_LIB = `pkg-config --libs "#{ularch}/pkgconfig/cairo.pc"`.strip
PANGO_CFLAGS = `pkg-config --cflags "#{ularch}/pkgconfig/pango.pc"`.strip
PANGO_LIB = `pkg-config --libs "#{ularch}/pkgconfig/pango.pc"`.strip

png_lib = 'png'

if ENV['DEBUG'] || ENV['GDB']
  LINUX_CFLAGS = " -g -O0"
else
#  LINUX_CFLAGS = " -O -Wall"
  LINUX_CFLAGS = " -O"
end
LINUX_CFLAGS << " -DRUBY_HTTP" 
LINUX_CFLAGS << " -DSHOES_GTK " 
LINUX_CFLAGS << " -DGTK3 " unless APP['GTK'] == 'gtk+-2.0'
LINUX_CFLAGS << xfixrvmp(`pkg-config --cflags "#{pkgruby}"`.strip)+" "
LINUX_CFLAGS << " -I#{TGT_SYS_DIR}usr/include/#{arch} "

LINUX_CFLAGS << xfixip("-I/usr/include")+" "
LINUX_CFLAGS << xfixip(`pkg-config --cflags "#{pkggtk}"`.strip)+" "
LINUX_CFLAGS << xfixip("-I/usr/include/librsvg-2.0/librsvg")+" "
MISC_LIB = '  /srv/chroot/debrpi/usr/lib/arm-linux-gnueabihf/librsvg-2.so'
LINUX_LIB_NAMES = %W[ungif jpeg]

DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared --sysroot=#{CHROOT} -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# dont use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "


LINUX_LIBS = "--sysroot=#{CHROOT} -L/usr/lib "
LINUX_LIBS << LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

# This chould be used in pre_build instead of 
# copy_deps_to_dist, although either would work. 
SOLOCS = {}
SOLOCS['ungif'] = "#{uldir}/libungif.so.4"
SOLOCS['gif'] = "#{uldir}/libgif.so.4"
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.8"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2"
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.0"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.0"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
