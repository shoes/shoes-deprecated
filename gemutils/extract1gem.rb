#!/usr/bin/env ruby
# example: $ ./extract1gem.rb ~/.rvm/gems/ruby-2.1.5/specifications/byebug-4.0.5.gemspec test/byebug-xarm6vhf/
# use when the CopyGem.rb won't look into . dot directories - sigh. 
require 'fileutils'
include FileUtils
specpath = ARGV[0]
destpath = ARGV[1]
spec = eval(File.read(specpath))
parts = specpath.split('/')

srcpath = File.join(parts[0..-3])
    if RUBY_PLATFORM =~ /mingw/
      srcpath.gsub!(/\\/, '/')  
      destpath.gsub!(/\\/, '/')  
    end


      puts "copy #{spec.full_name} from #{srcpath} to #{destpath}"
      mkdir_p(File.join(destpath, spec.full_name))
      cp File.join(srcpath, 'specifications', "#{spec.full_name}.gemspec") , 
         File.join(destpath, spec.full_name,'gemspec')
      # check for binary code
      rubyv = RUBY_VERSION[/\d.\d/]+'.0'
      gemcompl = File.join(srcpath, 'extensions', "#{Gem::Platform.local}",
         rubyv, spec.full_name, 'gem.build_complete')
      if File.exist? gemcompl
        puts "binary #{gemcompl}"
        cp gemcompl, File.join(destpath, spec.full_name)
      end 
      # copy lib/ or ext/ or whatever the spec says.
      # caution: spec is modified by the eval() so it's filled in with stuff
      puts "Require paths #{spec.require_paths}"
      skip1 = spec.require_paths
      if (skip1.length > 1) &&  (skip1.include? 'ext')
       # A confused gem! Me too. Perhaps Any binary gem?
       src = File.join(skip1)
       dest = File.join(destpath, spec.full_name, skip1[1])
       mkdir_p dest
       puts "weird ext copy #{src} -> #{dest}"
       cp_r src, dest
      elsif (skip1.length > 1) &&  (skip1.include? 'lib')
        puts "weird lib copy "
        cp_r File.join(srcpath,'gems', spec.full_name, 'lib'), File.join(destpath, spec.full_name)
      else
        skip1.each do |rqp| 
          puts "copy  #{rqp}"
          cp_r File.join(srcpath,'gems', spec.full_name, rqp), File.join(destpath, spec.full_name)
        end
      end

