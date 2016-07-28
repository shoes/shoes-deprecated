# some tests of debugging
puts "Starting #{ARGV} __FILE__ #{__FILE__}"
if ARGV.find_index('-d')
    ARGV.delete_if {|x| true}
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

