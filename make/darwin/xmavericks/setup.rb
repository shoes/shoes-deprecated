# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 
  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`} for #{TGT_DIR}"
	  rm_rf "#{TGT_DIR}"
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
	  # build a hash of x.dylib > BrewLoc/**/*.dylib
	  @brew_hsh = {}
	  Dir.glob("#{BREWLOC}/lib/**/*.dylib").each do |path|
		key = File.basename(path)
		@brew_hsh[key] = path
	  end
	
	  # Find ruby's dependent libs
	  cd "#{TGT_DIR}/lib/ruby/#{rbvm}.0/#{SHOES_TGT_ARCH}" do
		bundles = *Dir['*.bundle']
		puts "Bundles #{bundles}"
		cplibs = {}
		bundles.each do |bpath|
		  `otool -L #{bpath}`.split.each do |lib|
			cplibs[lib] = lib if File.extname(lib)=='.dylib'
		  end
		end
		cplibs.each_key do |k|
		  cppath = @brew_hsh[File.basename(k)]
		  if cppath
			cp cppath, "#{TGT_DIR}"
			chmod 0755, "#{TGT_DIR}/#{File.basename k}"
			puts "Copy #{cppath}"
		  else
			puts "Missing Ruby: #{k}"
		  end
		end
		# -id/-change the lib
		bundles.each do |f|
		  dylibs = get_dylibs f
		  dylibs.each do |dylib|
			if @brew_hsh[File.basename(dylib)]
			  sh "install_name_tool -change #{dylib} @executable_path/../#{File.basename dylib} #{f}"
			else
			  puts "Bundle lib missing #{dylib}"
			end
		  end
		end
		#abort "Quitting"
	  end
	end   
end

