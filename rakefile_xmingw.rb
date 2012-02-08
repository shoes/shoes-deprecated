require File.expand_path('make/xmingw/env')
require File.expand_path('make/xmingw/tasks')

rule ".o" => ".rc" do |t|
  MakeMinGW.make_resource t
end

