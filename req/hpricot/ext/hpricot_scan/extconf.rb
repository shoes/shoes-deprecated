if ENV['SYSROOT'] 
  rblv = ENV['TGT_RUBY_V']
  rbroot = ENV['TGT_RUBY_PATH']
  rlib = rbroot+"/lib"
  incl = "#{rbroot}/include/ruby-#{rblv}"
  incla = "#{incl}/#{ENV['TGT_ARCH']}"
  RbConfig::CONFIG["rubyhdrdir"] = incl
  RbConfig::CONFIG["rubyarchhdrdir"] = incla
  RbConfig::MAKEFILE_CONFIG['libdir'] = rlib # needed for Linking ext.so
  RbConfig::CONFIG['libdir'] = rlib # needed for conftest
end
require 'mkmf'
# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
$CFLAGS += ' -Wno-unused-function -Wno-unused-const-variable -Wno-shorten-64-to-32'
if ENV['SYSROOT']
  if ENV['TGT_RUBY_PATH'] =~ /mingw/
    $LDFLAGS = "-L #{rbroot}/bin"
    #puts "$LIBS = #{$LIBS}"
    $LIBS = ""
    CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']
  end
end
dir_config("hpricot_scan")
#have_library("c", "main")
create_makefile("hpricot_scan")
