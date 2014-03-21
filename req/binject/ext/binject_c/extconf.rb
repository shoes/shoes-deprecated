cross = ENV['SYSROOT']
if cross
  # We are cross compiling (arm or mingw)
  if ENV['TGT_RUBY_PATH'] !~ /mingw/
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
  else # mingw
    # paths are not chroot relative. The mingw compiled ruby knows 
    # where things are so we'll replace RbConfig::CONFIG and MAKEFILE_CONFIG 
    # save the running ruby path+name First
    bindir = RbConfig::CONFIG['bindir']
    rbname = RbConfig::CONFIG['ruby_install_name']
    require "#{ENV['EXT_RBCONFIG']}"  # causes complaints on terminal
    RbConfig::MAKEFILE_CONFIG['bindir'] = bindir
    RbConfig::MAKEFILE_CONFIG['ruby_install_name'] = rbname
    # Not all of the below lines are needed for mingw/binject. Doesn't hurt.
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
    RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC']
    RbConfig::MAKEFILE_CONFIG["rubyhdrdir"] = incl
    RbConfig::MAKEFILE_CONFIG["rubyarchhdrdir"] = incla
    RbConfig::MAKEFILE_CONFIG['libdir'] = rlib 
    RbConfig::MAKEFILE_CONFIG['rubylibdir'] = rlib 
  end
end

require 'mkmf'
# mkmf Puts things in CONFIG and $GLOBAL vars
#$CFLAGS += ' -Wno-declaration-after-statement -std=gnu99 -ffast-math'
if cross
  if ENV['TGT_RUBY_PATH'] =~ /mingw/
    #$LDFLAGS = "-L #{rbroot}/bin"
    #puts "$LIBS = #{$LIBS}"
    #$LIBS = ""
    #CONFIG['RUBY_SO_NAME'] = ENV['TGT_RUBY_SO']
    $CFLAGS << " -DSHOES_MINGW32 " 
  end
end

$CFLAGS << " -Iincludes "
$CFLAGS << "-DRUBY_#{RUBY_VERSION[0..2].sub(/\./,'_')} "
have_library("z")

if cross
  $CFLAGS << "--sysroot=#{ENV['SYSROOT']}"
end
create_makefile("binject")
