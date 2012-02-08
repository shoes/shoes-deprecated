# shoes.rb
$glbstr = "None"
begin
  require 'open-uri'
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

