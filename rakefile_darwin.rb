require 'make/rakefile_common'
require 'make/darwin/env'
require 'make/darwin/tasks'

task :stub do
  MakeDarwin.make_stub
end
