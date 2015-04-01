
# assuming arm, where the ruby was built with chroot relative paths
# set config vars to reflect where the running ruby can find things
rblv = ENV['TGT_RUBY_V']
rbroot = ENV['TGT_RUBY_PATH']
rlib = rbroot+"/lib"
incl = "#{rbroot}/include/ruby-#{rblv}"
incla = "#{incl}/#{ENV['TGT_ARCH']}"
# for the 'have' tests
RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC']
RbConfig::CONFIG["rubyhdrdir"] = incl
RbConfig::CONFIG["rubyarchhdrdir"] = incla
RbConfig::CONFIG['libdir'] = rlib 
# for building the ext (in the generated Makefile)
RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']
RbConfig::MAKEFILE_CONFIG["rubyhdrdir"] = incl
RbConfig::MAKEFILE_CONFIG["rubyarchhdrdir"] = incla
RbConfig::MAKEFILE_CONFIG['libdir'] = rlib 

puts "Loading sqlite3 mkmf"
require 'mkmf'

# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math '
$CFLAGS += "--sysroot=#{ENV['SYSROOT']}"
$LDFLAGS += " --sysroot=#{ENV['SYSROOT']}"

# :stopdoc:
#RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC'] # for compiling tests.
#RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

# --with-sqlite3-{dir,include,lib}

dir_config("sqlite3",
  ["#{ENV['SYSROOT']}/usr/include","#{ENV['SYSROOT']}/usr/include/arm-linux-gnueabihf"], 
  ["#{ENV['SYSROOT']}/usr/lib", "#{ENV['SYSROOT']}/lib/arm-linux-gnueabihf", "#{ENV['SYSROOT']}/usr/lib/arm-linux-gnueabihf"])


# prioritize local builds
#if enable_config("local", false)
#  $LDFLAGS = ENV.fetch("LDFLAGS", "")
#end


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

$CFLAGS << "--sysroot=#{ENV['SYSROOT']}"

create_makefile('sqlite3/sqlite3_native')
