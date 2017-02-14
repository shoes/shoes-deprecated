Shoes.setup do
  gem 'activerecord'
end

require 'active_record'

class Foo < ActiveRecord::Base; end

Shoes.app do
  para "it works"
end

