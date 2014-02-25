# envgem.rb -- Set things up for gem extension compile. It does not do the
# compile. It can only set Ruby vars and ENV vars. Do not assume Shoes
# constants exist - they probably don't. This script is require'd by
# the Shoes startup code in bin/main.skel[.c] for a --ruby <args> to
# process an extconf.rb (which will require mkmf.rb) DIR was set by
# the bin/main.skel[.c]
rblv = RUBY_VERSION < '2.0.0' ? '1.9.1' : RUBY_VERSION[/\d+.\d+/]+'.0'
incl = "#{DIR}/lib/ruby/include/ruby-#{rblv}"
incla = "#{incl}/#{RUBY_PLATFORM}"
# setup paths for ruby to load from
$: << "#{DIR}/lib/ruby/#{rblv}"
$: << "#{DIR}/lib/ruby/#{rblv}/#{RUBY_PLATFORM}"
#puts "#{$:}"
require 'rbconfig' # probably not needed
require 'fileutils'
include FileUtils
# now would be a good time to check if we have gcc and make. 
# For now, ASSUME we do. FIXME:
#$stderr.puts `which gcc`
# Below is required to find the ruby headers in mkmf.rb
RbConfig::CONFIG["rubyhdrdir"] = incl
# Below is used to define $ruby for install of gem
RbConfig::CONFIG["bindir"] = DIR
RbConfig::MAKEFILE_CONFIG['ruby_install_name'] = 'shoes --ruby'
# Below is a check to find libruby.so and set it. 
RbConfig::MAKEFILE_CONFIG['libdir'] = DIR # needed for Linking ext.so
RbConfig::CONFIG['libdir'] = DIR # needed for conftest
