# Only needed on windows (encoding.data not installed for linux/darwin)
if RbConfig::CONFIG['host_os'] =~ /mswin|mingw/
  IO.read(File.join DIR, 'encoding.data').
  each_line{|file| require File.join(DIR, 'ruby/lib/i386-mingw32/enc', file.chomp)}
end
