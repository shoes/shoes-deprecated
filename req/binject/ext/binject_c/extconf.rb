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
  RbConfig::CONFIG['CC'] = ENV['CC'] if ENV['CC']
end

require 'mkmf'
CONFIG.each do |k,v|
  puts "#{k}=#{v}" if v[/rvm/]
end
#$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
if ENV['SYSROOT']
  if ENV['TGT_RUBY_PATH'] =~ /mingw/
    $LDFLAGS = "-L #{rbroot}/bin"
    #puts "$LIBS = #{$LIBS}"
    $LIBS = ""
    CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']
    $CFLAGS << "-DSHOES_MINGW32 " 
  else # FIXME: assume raspberry pi build
    oslib = "#{ENV['SYSROOT']}/usr/lib"
    oslibarch = oslib+'/arm-linux-gnueabihf'
    $LDFLAGS = "-L#{oslib} -L#{oslibarch}"
    CONFIG['prefix'] = ENV['SYSROOT']
  end
end

$CFLAGS << " -Iincludes "
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
have_library("z")
create_makefile("binject")
