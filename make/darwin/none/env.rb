# just enough allow a rake -T
EXT_RUBY = File.exists?("deps/ruby") ? "deps/ruby" : RbConfig::CONFIG['prefix']
CC = "gcc"
DLEXT = "dll"
LINUX_CFLAGS = []

