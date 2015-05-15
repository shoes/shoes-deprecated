Some scripts for moving gems out of an installed ruby (like ~/.rvm/gem/...)
to a directory where they can be tarred up. Not everything in the gem is copied.just lib, ext in rare cases, the gemspec and gem.build_complete if its a binary gem. Shoes has no use for tests, benchmarks, rdoc. This can make the gem's smaller - much smaller for some gems. 
