require File.expand_path('make/rakefile_common')
require File.expand_path('make/mingw/env')
require File.expand_path('make/mingw/tasks')

rule ".o" => ".rc" do |t|
  MakeMinGW.make_resource t
end
