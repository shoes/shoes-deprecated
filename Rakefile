require 'rubygems'
require 'bundler'
Bundler::GemHelper.install_tasks

require 'rake/testtask'
Rake::TestTask.new do |t|
  t.pattern = "test/*_test.rb" 
end 

require 'rake/extensiontask'

Rake::ExtensionTask.new('shoes') do |ext|
  ext.lib_dir = File.join('lib', 'shoes')
end

