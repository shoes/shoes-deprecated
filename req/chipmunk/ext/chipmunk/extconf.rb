require 'mkmf'
# update the CONFIG with the correct values. RbConfig won't work 
# for cross compiling. This is a bit heavy handed. There might be a better way.
# OK. With the mingw stuff, it's a hack. 
CONFIG['CC']=ENV['CC'] if ENV['CC']
$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
if ENV['SYSROOT']
  rbroot = ENV['TGT_RUBY_PATH']
  #puts "Using rbroot=#{rbroot} instead of #{$hdrdir}"
  $hdrdir = "#{rbroot}/include/ruby-#{ENV['TGT_RUBY_V']}"
  $topdir = $hdrdir
  $arch_hdrdir = "#{$hdrdir}/#{ENV['TGT_ARCH']}"
  CONFIG['prefix'] = "#{rbroot}"
  if rbroot =~ /mingw32/
    $LDFLAGS = "-L #{rbroot}/bin"
    #puts "$LIBS = #{$LIBS}"
    $LIBS = ""
    CONFIG['RUBY_SO_NAME'] = "msvcrt-ruby191"
  end
end
create_makefile('chipmunk')
