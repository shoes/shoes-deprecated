# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 

  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`}"
    rbvt = RUBY_V
    rbvm = RUBY_V[/^\d+\.\d+/]
    mkdir_p "#{TGT_DIR}/lib"
    mkdir_p "#{TGT_DIR}/fonts"
    cp_r "fonts", "#{TGT_DIR}/fonts"
    mkdir_p "#{TGT_DIR}/lib"
    cp   "lib/shoes.rb", "#{TGT_DIR}/lib"
    cp_r "lib/shoes", "#{TGT_DIR}/lib"
    cp_r "lib/exerb", "#{TGT_DIR}/lib"
    cp_r "samples", "#{TGT_DIR}/samples"
    cp_r "static", "#{TGT_DIR}/static"
    cp   "README.md", "#{TGT_DIR}/README.txt"
    cp   "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp   "COPYING", "#{TGT_DIR}/COPYING.txt"
    # clean out leftovers from last build
    #rm_f "#{TGT_DIR}/libruby.so" if File.exist? "#{TGT_DIR}/libruby.so"
    #rm_f "#{TGT_DIR}/libruby.so.#{rbvm}" if File.exist? "#{TGT_DIR}/libruby.so.#{rbvm}"
    #rm_f "#{TGT_DIR}/libruby.so.#{rbvt}" if File.exist? "#{TGT_DIR}/libruby.so.#{rbvt}"
    cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib", remove_destination:  true
    # copy and link libruby.so
    cp "#{EXT_RUBY}/lib/libruby.so.#{rbvm}", "#{TGT_DIR}"

    # copy include files - it might help build gems
    mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
    cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
    chdir TGT_DIR do
      ln_s "libruby.so.#{rbvm}", "libruby.so"
    end   
    SOLOCS.each_value do |path|
      cp "#{path}", "#{TGT_DIR}"
    end
    chdir TGT_DIR do
      SYMLNK.each do |k, v|
        v.each do |syml|
          ln_s k, syml
        end
      end
    end
  end
end

