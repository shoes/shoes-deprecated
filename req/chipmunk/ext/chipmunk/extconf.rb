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
  if RUBY_PLATFORM =~ /darwin/
    ENV['ARCH_FLAG'] = ENV['SYSROOT']
  end
end
require 'mkmf'
# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. There might be a better way.
# OK. With the mingw stuff, it's a hack. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
if ENV['SYSROOT']
  CONFIG['prefix'] = "#{rbroot}"
  if rbroot =~ /mingw/
    $LDFLAGS = "-L #{rbroot}/bin"
    #puts "$LIBS = #{$LIBS}"
    $LIBS = ""
    CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']
  end
end
create_makefile('chipmunk')
