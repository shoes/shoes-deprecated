# -*- encoding: utf-8 -*-
# stub: rubygems-update 2.6.8 ruby hide_lib_for_update

Gem::Specification.new do |s|
  s.name = "rubygems-update"
  s.version = "2.6.8"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.require_paths = ["hide_lib_for_update"]
  s.authors = ["Jim Weirich", "Chad Fowler", "Eric Hodel"]
  s.date = "2016-10-29"
  s.description = "RubyGems is a package management framework for Ruby.\n\nThis gem is an update for the RubyGems software. You must have an\ninstallation of RubyGems before this update can be applied.\n\nSee Gem for information on RubyGems (or `ri Gem`)\n\nTo upgrade to the latest RubyGems, run:\n\n  $ gem update --system  # you might need to be an administrator or root\n\nSee UPGRADING.rdoc for more details and alternative instructions.\n\n-----\n\nIf you don't have RubyGems installed, you can still do it manually:\n\n* Download from: https://rubygems.org/pages/download, unpack, and cd there\n* OR clone this repository and cd there\n* Install with: ruby setup.rb  # you may need admin/root privilege\n\nFor more details and other options, see:\n\n  ruby setup.rb --help"
  s.email = ["rubygems-developers@rubyforge.org"]
  s.executables = ["update_rubygems"]
  s.extra_rdoc_files = ["CODE_OF_CONDUCT.md", "CONTRIBUTING.rdoc", "CVE-2013-4287.txt", "CVE-2013-4363.txt", "CVE-2015-3900.txt", "History.txt", "LICENSE.txt", "MAINTAINERS.txt", "MIT.txt", "Manifest.txt", "POLICIES.rdoc", "README.rdoc", "UPGRADING.rdoc", "bundler/CHANGELOG.md", "bundler/CODE_OF_CONDUCT.md", "bundler/CONTRIBUTING.md", "bundler/DEVELOPMENT.md", "bundler/ISSUES.md", "bundler/LICENSE.md", "bundler/README.md", "bundler/man/index.txt", "hide_lib_for_update/note.txt"]
  s.files = ["CODE_OF_CONDUCT.md", "CONTRIBUTING.rdoc", "CVE-2013-4287.txt", "CVE-2013-4363.txt", "CVE-2015-3900.txt", "History.txt", "LICENSE.txt", "MAINTAINERS.txt", "MIT.txt", "Manifest.txt", "POLICIES.rdoc", "README.rdoc", "UPGRADING.rdoc", "bin/update_rubygems", "bundler/CHANGELOG.md", "bundler/CODE_OF_CONDUCT.md", "bundler/CONTRIBUTING.md", "bundler/DEVELOPMENT.md", "bundler/ISSUES.md", "bundler/LICENSE.md", "bundler/README.md", "bundler/man/index.txt", "hide_lib_for_update/note.txt"]
  s.homepage = "https://rubygems.org"
  s.licenses = ["Ruby", "MIT"]
  s.rdoc_options = ["--main", "README.rdoc", "--title=RubyGems Update Documentation"]
  s.required_ruby_version = Gem::Requirement.new(">= 1.8.7")
  s.rubygems_version = "2.5.1"
  s.summary = "RubyGems is a package management framework for Ruby"

  s.installed_by_version = "2.5.1" if s.respond_to? :installed_by_version

  if s.respond_to? :specification_version then
    s.specification_version = 4

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<builder>, ["~> 2.1"])
      s.add_development_dependency(%q<hoe-seattlerb>, ["~> 1.2"])
      s.add_development_dependency(%q<rdoc>, ["~> 3.0"])
      s.add_development_dependency(%q<ZenTest>, ["~> 4.5"])
      s.add_development_dependency(%q<rake>, ["~> 10.5"])
      s.add_development_dependency(%q<minitest>, ["~> 4.0"])
      s.add_development_dependency(%q<hoe>, ["~> 3.15"])
    else
      s.add_dependency(%q<builder>, ["~> 2.1"])
      s.add_dependency(%q<hoe-seattlerb>, ["~> 1.2"])
      s.add_dependency(%q<rdoc>, ["~> 3.0"])
      s.add_dependency(%q<ZenTest>, ["~> 4.5"])
      s.add_dependency(%q<rake>, ["~> 10.5"])
      s.add_dependency(%q<minitest>, ["~> 4.0"])
      s.add_dependency(%q<hoe>, ["~> 3.15"])
    end
  else
    s.add_dependency(%q<builder>, ["~> 2.1"])
    s.add_dependency(%q<hoe-seattlerb>, ["~> 1.2"])
    s.add_dependency(%q<rdoc>, ["~> 3.0"])
    s.add_dependency(%q<ZenTest>, ["~> 4.5"])
    s.add_dependency(%q<rake>, ["~> 10.5"])
    s.add_dependency(%q<minitest>, ["~> 4.0"])
    s.add_dependency(%q<hoe>, ["~> 3.15"])
  end
end
