# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'
puts `pwd`
require '../lib/shoes/dataseries/csvseries.rb'


class CsvSeriesTest < Test::Unit::TestCase

  def setup 
   @csv = CsvSeries.create('Tests/tstest.csv')
  end
  
  def test_name
    assert_equal 'tstest', @csv.name
  end
end


Shoes.app title: "Testing Timeseries (csv)" do
    
    tests = Test::Unit::TestSuite.new("Timeseries Tests")
    [CsvSeriesTest].each { |t| tests << t.suite }
    
    Shoes.terminal
    @test_result = Test::Unit::UI::Console::TestRunner.run(tests, {use_color: true})
end
