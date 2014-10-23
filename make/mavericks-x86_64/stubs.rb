desc "Build stubs (OSX cross compile only)"
task :stubs  do
  #ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
  #sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
  #sh "gcc -O -g3 -isysroot #{OSX_SDK} -framework Cocoa -o static/stubs/shoes-osx-install platform/mac/stub.m -I."
  sh "gcc -O -isysroot #{OSX_SDK} -framework Cocoa -o static/stubs/shoes-osx-install platform/mac/osxdnl.m -I."
end
