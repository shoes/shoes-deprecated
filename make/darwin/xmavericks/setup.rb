# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 
  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`} for #{TGT_DIR}"
	  #rm_rf "#{TGT_DIR}"
    mkdir_p "#{TGT_DIR}/#{APP['Bld_Tmp']}"
	  # copy Ruby, dylib, includes - have them in place before
	  # we build exts (chipmunk).
	  puts "Ruby at #{EXT_RUBY}"
	  rbvt = RUBY_V
	  rbvm = RUBY_V[/^\d+\.\d+/]
	  mkdir_p "#{TGT_DIR}/lib"
	  # clean out leftovers from last build
	  rm_f "#{TGT_DIR}/libruby.dylib" if File.exist? "#{TGT_DIR}/libruby.dylib"
	  rm_f "#{TGT_DIR}/libruby.#{rbvm}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvm}.dylib"
	  rm_f "#{TGT_DIR}/libruby.#{rbvt}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvt}.dylib"
	  mkdir_p "#{TGT_DIR}/lib/ruby/#{rbvm}.0/#{RUBY_PLATFORM}"
	  cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
	  # copy and link libruby.dylib
	  cp "#{EXT_RUBY}/lib/libruby.#{rbvt}.dylib", "#{TGT_DIR}"
	  # copy include files - it might help build gems
	  mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
	  cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
	  SOLOCS.each do |k, v| 
	    cp v, TGT_DIR
	  end
    
    # copy some static stuff
    cp_r  "fonts", "#{TGT_DIR}/fonts"
    cp_r  "lib", "#{TGT_DIR}"
    cp_r  "samples", "#{TGT_DIR}/samples"
    cp_r  "static", "#{TGT_DIR}/static"
    cp    "README.md", "#{TGT_DIR}/README.txt"
    cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
	end
end

