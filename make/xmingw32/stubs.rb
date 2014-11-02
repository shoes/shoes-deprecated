# build stubs - Much disappointment below.
# 
desc "Build stubs (MinGW cross compile only)"
task :stubs  do
  CPP = "i686-w64-mingw32-g++"
  Dir.chdir("shoes/http") do
    sh "#{CC} #{LINUX_CFLAGS} -I../../ -c winhttp.c"
  end
  Dir.chdir("platform/msw") do
    sh "#{CC} #{ENV['GDB'] ? ' -DSTUB_DEBUG ' : ' '} #{LINUX_CFLAGS} -I../../ -c stub.c"
    sh "i686-w64-mingw32-windres  -o stub32.o -i stub32.rc"
    sh "#{CC} -o shoes-stub.exe  stub.o  stub32.o ../../shoes/http/winhttp.o -lwinhttp -lcomctl32 #{ENV['GDB'] ? ' ' : '-mwindows'} "
    sh "i686-w64-mingw32-strip -d shoes-stub.exe"
    sh "cp shoes-stub.exe ../../static/stubs/"
  end
end

