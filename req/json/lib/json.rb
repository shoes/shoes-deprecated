# stub to enforce use of the C extension.
require 'json/common'

module JSON
  require 'json/version'
  require 'json/ext'
  JSON_LOADED = true
end
