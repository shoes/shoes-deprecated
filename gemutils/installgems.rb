# ruby script to copy all gems in a directory to Shoes gem dir.
srcdir = "E:/gems/test/xwin7"
dest = "C:/Users/Cecil/AppData/Local/Shoes/+gem"
require 'rubygems'
gems = []
require 'FileUtils'
include FileUtils
gems = Dir.glob(File.join(srcdir, '*'))
mkdir_p dest
gems.each do |gempath|
  # look inside for the gem.build_complete
  gemn = File.split(gempath)[1]
  if File.exists? File.join(gempath,'gem.build_complete')
    extpath = File.join(dest, 'extensions', "#{Gem::Platform.local}", '2.1.0', gemn)
    puts extpath
    mkdir_p extpath
    cp File.join(gempath,'gem.build_complete'), extpath
  end
  # copy the gemspec
  specpath = File.join(dest, 'specifications')
  mkdir_p specpath
  specname = gemn+'.gemspec'
  cp File.join(gempath,'gemspec'), File.join(specpath, specname)
  # copy ext if we have one
  if File.exists? File.join(gempath, 'ext')
    puts "Copy ext #{gempath}"
    mkdir_p File.join(dest, 'gems', gemn)
    cp_r File.join(gempath, 'ext'), File.join(dest,'gems', gemn)
  end
  # copy lib if we have it
  if File.exists? File.join(gempath, 'lib')
    mkdir_p File.join(dest, 'gems', gemn)
    cp_r File.join(gempath, 'lib'), File.join(dest,'gems', gemn)
  end
end
