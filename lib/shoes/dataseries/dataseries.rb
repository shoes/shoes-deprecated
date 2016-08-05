class DataSeries
  attr_accessor :values, :type, :ary, :startpt, :endpt, :minv, :maxv, :short_name
  def initialize(short_name, boolv, ary, values, startpt, endpt, minv, maxv)
    @short_name = short_name
    @type = boolv
    @values = values
    @ary = ary
    @startpt = startpt
    @endpt = endpt
    @minv = minv
    @maxv = maxv
  end
  
  def at(key)
    return 
  end
  
end
