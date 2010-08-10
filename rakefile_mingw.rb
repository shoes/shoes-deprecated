require 'make/rakefile_common'
require 'make/mingw/env'
require 'make/mingw/tasks'

rule ".o" => ".rc" do |t|
  MakeMinGW.make_resource t
end
