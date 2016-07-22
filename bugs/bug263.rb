# some tests of debugging
# next over: 
puts require 'irb'
puts require 'pry'
puts "Starting #{ARGV} __FILE__ #{__FILE__}"
if ARGV.find_index('-me')
    require 'byebug'
    byebug
end
one = 1
two = "two"

Shoes.app do
  stack do
    para one
    para two
    flow do
      button "rdb" do
        require 'byebug'
        byebug
        para "rdb: #{one} #{two}"
      end
    end
  end
end

