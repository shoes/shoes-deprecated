# package ShoesDeps into just the minimum to upload
# expect symlink trouble with include/x --> lib/x/include/
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
  sh "cp -a #{ShoesDeps}/include mingwdeps"
  cp_r "#{ShoesDeps}/lib", 'mingwdeps'
  cp_r "#{ShoesDeps}/share", 'mingwdeps'
  Dir.chdir('mingwdeps') do
   sh "zip ShoesDep.zip README.txt lib/* share/* include/* bin/*"
  end
  puts "Done"
end
