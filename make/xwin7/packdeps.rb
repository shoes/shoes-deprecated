# package ShoesDeps into just the minimum to upload
# expect symlink trouble with include/x --> lib/x/include/ ?
desc "package mingw dependencies"
task :packdeps  do
  # ugly - build a ShoesDeps.zip in dist
  rm_rf 'mingwdeps'
  mkdir_p 'mingwdeps'
  cp "#{ShoesDeps}/README.txt", 'mingwdeps'
  bin = 'mingwdeps/bin'
  mkdir_p bin
  Dir.glob("#{ShoesDeps}/bin/*.dll") { |f|
    cp f, bin
  }
  Dir.glob("#{ShoesDeps}/bin/fc*.exe") {|f|
    cp f, bin
  }
  cp "#{ShoesDeps}/bin//gtk-update-icon-cache.exe", bin
  cp "#{ShoesDeps}/bin/pkg-config.exe", bin
  sh "cp -a #{ShoesDeps}/include mingwdeps"
  cp_r "#{ShoesDeps}/etc", 'mingwdeps'
  cp_r "#{ShoesDeps}/lib", 'mingwdeps'
  mkdir_p 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/fontconfig", 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/glib-2.0", 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/themes", 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/xml", 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/fontconfig", 'mingwdeps/share'
  cp_r "#{ShoesDeps}/share/icons", 'mingwdeps/share'
  Dir.chdir('mingwdeps') do
   sh "zip -r ShoesDeps.zip README.txt etc lib share include bin"
  end
  puts "Done"
end
