Gem::Specification.new do |s|
  s.name = "psych"
  s.version = "2.0.5"
  s.date = "2014-03-27"
  s.summary = "Psych is a YAML parser and emitter"
  s.description = "Psych is a YAML parser and emitter.  Psych leverages libyaml[http://pyyaml.org/wiki/LibYAML]\nfor its YAML parsing and emitting capabilities.  In addition to wrapping\nlibyaml, Psych also knows how to serialize and de-serialize most Ruby objects\nto and from the YAML format."
  s.homepage = "http://github.com/tenderlove/psych"
  s.authors = ["Aaron Patterson"]
  s.email = ["aaron@tenderlovemaking.com"]
  s.files = ["psych.rb", "psych.so", "psych/class_loader.rb", "psych/coder.rb", "psych/core_ext.rb", "psych/deprecated.rb", "psych/exception.rb", "psych/handler.rb", "psych/handlers/document_stream.rb", "psych/handlers/recorder.rb", "psych/json/ruby_events.rb", "psych/json/stream.rb", "psych/json/tree_builder.rb", "psych/json/yaml_events.rb", "psych/nodes.rb", "psych/nodes/alias.rb", "psych/nodes/document.rb", "psych/nodes/mapping.rb", "psych/nodes/node.rb", "psych/nodes/scalar.rb", "psych/nodes/sequence.rb", "psych/nodes/stream.rb", "psych/omap.rb", "psych/parser.rb", "psych/scalar_scanner.rb", "psych/set.rb", "psych/stream.rb", "psych/streaming.rb", "psych/syntax_error.rb", "psych/tree_builder.rb", "psych/visitors.rb", "psych/visitors/depth_first.rb", "psych/visitors/emitter.rb", "psych/visitors/json_tree.rb", "psych/visitors/to_ruby.rb", "psych/visitors/visitor.rb", "psych/visitors/yaml_tree.rb", "psych/y.rb"]
end
