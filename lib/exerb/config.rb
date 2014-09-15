
#==============================================================================#
# $Id: config.rb,v 1.23 2008/06/13 23:54:45 arton Exp $
#==============================================================================#

require 'rbconfig'

#==============================================================================#

module Exerb
  ver = RUBY_VERSION.gsub('.','')

  # Search directories of a core.
  # If running exerb on exerb, Add self path to the search directories of a core.
  CORE_PATH = [
    (File.dirname(ExerbRuntime.filepath) if defined?(ExerbRuntime)),
    ENV['EXERBCORE'],
    File.join(Config::CONFIG['datadir'], 'exerb'),
    '.',
  ].compact

  # Name definitions of a core.
  CORE_NAME = {
    'cui'    => "ruby#{ver}c.exc",
    'cuid'   => "ruby#{ver}cd.exc",
    'cuirt'  => "ruby#{ver}crt.exc",
    'cuirtd' => "ruby#{ver}crtd.exc",
    'gui'    => "ruby#{ver}g.exc",
    'guid'   => "ruby#{ver}gd.exc",
    'guirt'  => "ruby#{ver}grt.exc",
    'guirtd' => "ruby#{ver}grtd.exc",
  }

  # Descriptions of a core.
  CORE_DESC = {
    # FIXME: Add descriptions
    # "ruby#{ver}c.exc" => '...',
  }

end # Exerb

#==============================================================================#

# Load a configuration file of Exerb Core Collection if found that.
configcc = File.join(File.dirname(__FILE__), 'configcc.rb')
if File.exist?(configcc)
  require(configcc)
end

#==============================================================================#
#==============================================================================#
