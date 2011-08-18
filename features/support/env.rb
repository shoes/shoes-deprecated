require 'rspec/mocks'

class Shoes;
  RSpec::Mocks.setup(self)
  class Types; end
  class Background; end
  class Border; end
  class Canvas; end
  class Check; end
  class Radio; end
  class EditLine; end
  class EditBox; end
  class Effect; end
  class Image; end
  class ListBox; end
  class Progress; end
  class Shape; end
  class TextBlock; end
  class Text; end

  # All of the classes above are ones that Shoes needs to be just
  # bootstrapped, all the ones below are ones I'm mocking out from
  # the C code. I'm not sure if there's a good distinction...

  class Button
    attr_accessor :name
    def initialize(name, &blk)
      self.name = name
      instance_eval &blk if block_given?
    end
  end

  attr_accessor :elements

  def initialize
    self.elements = []
  end

  def self.app
    new
  end

  def append(&blk)
    instance_eval &blk if block_given?
  end

  def button(name, &blk)
    self.elements << Button.new(name, &blk)
  end
end

DIR = Dir.pwd

require_relative '../../lib/shoes'
