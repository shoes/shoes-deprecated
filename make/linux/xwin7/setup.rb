# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 
  #  Windows cross compile.  Copy the static stuff, Copy the ruby libs
  #  Then copy the deps.
  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`}"
    rbvt = RUBY_V
    rbvm = RUBY_V[/^\d+\.\d+/]
    # remove leftovers from previous rake.
    rm_rf "#{TGT_DIR}/lib"
    rm_rf "#{TGT_DIR}/etc"
    rm_rf "#{TGT_DIR}/share"
    rm_rf "#{TGT_DIR}/conf.d"
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
    #mkdir_p "#{TGT_DIR}/lib"
    cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib", remove_destination: true
    # copy include files
    mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
    cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
    so_list.each_value do |path|
      cp "#{path}", "#{TGT_DIR}"
    end
    # copy/setup etc/share
    mkdir_p "#{TGT_DIR}/share/glib-2.0/schemas"
    cp  "#{ShoesDeps}/share/glib-2.0/schemas/gschemas.compiled" ,
        "#{TGT_DIR}/share/glib-2.0/schemas"
    cp_r "#{ShoesDeps}/share/fontconfig", "#{TGT_DIR}/share"
    cp_r "#{ShoesDeps}/share/themes", "#{TGT_DIR}/share"
    cp_r "#{ShoesDeps}/share/xml", "#{TGT_DIR}/share"
    cp_r "#{ShoesDeps}/share/icons", "#{TGT_DIR}/share"
    sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
    cp_r "#{ShoesDeps}/etc", TGT_DIR
    if ENABLE_MS_THEME
      ini_path = "#{TGT_DIR}/etc/gtk-3.0"
      mkdir_p ini_path
      File.open "#{ini_path}/settings.ini", mode: 'w' do |f|
        f.write "[Settings]\n"
        f.write "#gtk-theme-name=win32\n"
      end
    end
    mkdir_p "#{ShoesDeps}/lib"
    cp_r "#{ShoesDeps}/lib/gtk-3.0", "#{TGT_DIR}/lib" 
    bindir = "#{ShoesDeps}/bin"
    if File.exist?("#{bindir}/gtk-update-icon-cache-3.0.exe")
      cp "#{bindir}/gtk-update-icon-cache-3.0.exe",
            "#{TGT_DIR}/gtk-update-icon-cache.exe"
    else 
      cp  "#{bindir}/gtk-update-icon-cache.exe", TGT_DIR
    end
    cp APP['icons']['win32'], "shoes/appwin32.ico"
  end
end

