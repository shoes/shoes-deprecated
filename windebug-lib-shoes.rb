# shoes.rb
ARGV.delete_if { |x| x =~ /-psn_/ }

class Encoding
  %w[UTF_7 UTF_16BE UTF_16LE UTF_32BE UTF_32LE].each do |enc|
    eval "class #{enc};end" unless const_defined? enc.to_sym
  end
end

$glbstr = "None"
begin
  require 'open-uri'
  require 'optparse'
  require 'resolv-replace' if RUBY_PLATFORM =~ /win/
  require_relative 'shoes/inspect'
  require_relative 'shoes/cache'
  #if Object.const_defined? :Shoes
  #  require_relative 'shoes/image'
  #end
  #require 'shoes/shybuilder'

rescue Exception => e
  $glbstr = e.message
end

class Shoes
  def self.run path
    [nil]
  end
  def self.args!
    Shoes.app do
        para "#{RUBY_PLATFORM}\n"
        para "#{$glbstr}\n"
        button "Quit" do
          exit
        end
    end
  end
end

