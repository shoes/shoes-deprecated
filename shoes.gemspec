# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)
require "shoes/version"

Gem::Specification.new do |s|
  s.name        = "shoes"
  s.version     = Shoes::VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["Steve Klabnik", "Team Shoes"]
  s.email       = ["steve@steveklabnik.com"]
  s.homepage    = "http://github.com/shoes/shoes"
  s.summary     = %q{Shoes is the best little GUI toolkit for Ruby.}
  s.description = %q{Shoes is the best little GUI toolkit for Ruby. This gem is currently a placeholder until we properly gemfiy Shoes.}

  s.add_development_dependency "mechanize"
  s.add_development_dependency "rake-compiler"

  #s.add_dependency "bloopsaphone"
  s.add_dependency "chipmunk"
  s.add_dependency "hpricot"
  s.add_dependency "json"
  s.add_dependency "sqlite3"
  s.add_dependency "redcarpet"

  s.extensions << "ext/shoes/extconf.rb"

  s.files         = `git ls-files`.split("\n") + ["lib/shoes/shoes.bundle"]
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  #s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]
end
