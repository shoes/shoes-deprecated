# cross compile for win7 on linux
# save the running ruby path+name First
bindir = RbConfig::CONFIG['bindir']
rbname = RbConfig::CONFIG['ruby_install_name']
# remove RbConfig:: constants
RbConfig::send :remove_const, :TOPDIR
RbConfig::send :remove_const, :CONFIG
RbConfig::send :remove_const, :MAKEFILE_CONFIG
# puts "load new consts from #{ENV['EXT_RBCONFIG']}"
load "#{ENV['EXT_RBCONFIG']}"
# restore running ruby path and name
RbConfig::CONFIG['bindir'] = bindir
RbConfig::MAKEFILE_CONFIG['bindir'] = bindir
RbConfig::CONFIG['ruby_install_name'] = rbname
RbConfig::MAKEFILE_CONFIG['ruby_install_name'] = rbname
# Not all of the below lines are needed. Doesn't hurt to make sure.
rblv = ENV['TGT_RUBY_V']
rbroot = ENV['TGT_RUBY_PATH']
rlib = rbroot+"/bin"
incl = "#{rbroot}/include/ruby-#{rblv}"
incla = "#{incl}/#{ENV['TGT_ARCH']}"
# for the 'have' tests
RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC']
RbConfig::CONFIG["rubyhdrdir"] = incl
RbConfig::CONFIG["rubyarchhdrdir"] = incla
RbConfig::CONFIG['libdir'] = rlib 
RbConfig::CONFIG['rubylibdir'] = rlib 
# for building the ext (in the generated Makefile)
RbConfig::MAKEFILE_CONFIG["rubyhdrdir"] = incl
RbConfig::MAKEFILE_CONFIG["rubyarchhdrdir"] = incla
RbConfig::MAKEFILE_CONFIG['libdir'] = rlib 
RbConfig::MAKEFILE_CONFIG['rubylibdir'] = rlib 

puts "Loading sqlite3 mkmf"
require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math '
$LDFLAGS = "-L #{rbroot}/lib "
$LIBS = ""
CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']

# :stopdoc:
#RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC'] # for compiling tests.
#RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

# --with-sqlite3-{dir,include,lib}
dir_config("sqlite3","#{ENV['SYSROOT']}/include", ["#{ENV['SYSROOT']}/bin"])


if RbConfig::CONFIG["host_os"] =~ /mswin/
  $CFLAGS << ' -W3'
end

def asplode missing
  if RUBY_PLATFORM =~ /mingw|mswin/
    abort "#{missing} is missing. Install SQLite3 from " +
          "http://www.sqlite.org/ first."
  else
    abort <<-error
#{missing} is missing. Try 'port install sqlite3 +universal',
'yum install sqlite-devel' or 'apt-get install libsqlite3-dev'
and check your shared library search path (the
location where your sqlite3 shared library is located).
    error
  end
end

asplode('sqlite3.h')  unless find_header  'sqlite3.h'
asplode('sqlite3') unless find_library 'sqlite3', 'sqlite3_libversion_number'

# Functions defined in 1.9 but not 1.8
have_func('rb_proc_arity')

# Functions defined in 2.1 but not 2.0
have_func('rb_integer_pack')

# These functions may not be defined
have_func('sqlite3_initialize')
have_func('sqlite3_backup_init')
have_func('sqlite3_column_database_name')
have_func('sqlite3_enable_load_extension')
have_func('sqlite3_load_extension')
have_func('sqlite3_open_v2')
have_func('sqlite3_prepare_v2')
have_type('sqlite3_int64', 'sqlite3.h')
have_type('sqlite3_uint64', 'sqlite3.h')
#if cross
#  $CFLAGS << "--sysroot=#{ENV['SYSROOT']}"
#end
create_makefile('sqlite3/sqlite3_native')
