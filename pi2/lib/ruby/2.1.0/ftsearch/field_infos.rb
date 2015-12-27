# Copyright (C) 2006  Mauricio Fernandez <mfp@acm.org>
#

require 'ftsearch/analysis/whitespace_analyzer'

module FTSearch
class FieldInfos
  DEFAULT_OPTIONS = {
    :default_analyzer => FTSearch::Analysis::WhiteSpaceAnalyzer.new,
    :stored => true,
  }
  def initialize(options = {})
    options = DEFAULT_OPTIONS.merge(options)
    @fields = {}
    @default_options = options
  end

  def add_field(options = {})
    options = @default_options.merge(options)
    raise "Need a name" unless options[:name]
    store_field_info(options)
  end

  def [](field_name)
    if field_info = @fields[field_name]
      field_info
    else
      store_field_info(:name => field_name)
    end
  end

  private
  def store_field_info(options)
    options = @default_options.merge(options)
    unless options[:analyzer]
      if klass = options[:analyzer_class]
        options[:analyzer] = klass.new
      else
        options[:analyzer] = @default_options[:default_analyzer]
      end
    end
    @fields[options[:name]] = options
  end
end
end  # FTSearch

