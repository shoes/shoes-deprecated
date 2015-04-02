# xsnow is type of cross compile
rblv = ENV['TGT_RUBY_V']
rbroot = ENV['TGT_RUBY_PATH']
rlib = rbroot+"/lib"
incl = "#{rbroot}/include/ruby-#{rblv}"
incla = "#{incl}/#{ENV['TGT_ARCH']}"
RbConfig::CONFIG["rubyhdrdir"] = incl
RbConfig::CONFIG["rubyarchhdrdir"] = incla
RbConfig::MAKEFILE_CONFIG['libdir'] = rlib # needed for Linking ext.so
RbConfig::CONFIG['libdir'] = rlib # needed for conftest
ARCH_FLAG = ENV['SYSROOT']

require 'mkmf'

CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-shorten-64-to-32 -Wno-return-type -Wno-unused-variable'
$CFLAGS += ' -Wno-implicit-function-declaration -Wno-unused-function'

#have_header('stdio.h') or exit
dir_config('fast_xs')
create_makefile('fast_xs')

