require File.expand_path('make/rakefile_common')
require File.expand_path('make/darwin/env')
require File.expand_path('make/darwin/tasks')

task :stub do
  MakeDarwin.make_stub
end
