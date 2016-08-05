class DataSeries
  attr_accessor :values, :type, :ary, :startpt, :endpt, :minv, :maxv, :short_name
  def initialize(short_name, boolv, ary, values, startpt, endpt, minv, maxv)
    @short_name = short_name
    @type = boolv
    @values = values # hash
    @ary = ary       # array
    @startpt = startpt
    @endpt = endpt
    @minv = minv
    @maxv = maxv
  end
  
  # return value at Datetime (hash key]
  def valueAtTime(dt)
    return @values[dt]
  end
  
  # return value at idx
  def valueAtIndex(idx)
    return @ary[idx]
  end
  
end
