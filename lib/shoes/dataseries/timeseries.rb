# note: this could/should become a C based implmentation so don't get too
# clever with Ruby
class TimeSeries
  attr_accessor :bool, :startpt, :endpt, :minv, :maxv, :short_name
  def initialize(short_name)
    @short_name = short_name
    @bool = true # true for boolean series (floats 0.0 and 1.0)
    @values = []  # array of floats
    @dthash  = {} # datetime to index in @values.
    @startpt = 0  # replaced with a DateTime
    @endpt = 0    # replaced with a DateTime
    @maxv = 0.0
    @minv = 100000.0 # Float.MAX doesn't work
    @idx = 0
  end
  
  #this is the heavy lifter (expanding arrays and hashes)
  def append(dt, v)
    @maxv = v if v > @maxv
    @minv = v if v < @minv
    if @idx == 0
      @startpt = dt
    else
      @endpt = dt
    end
    @bool = false if v != 0.0 or v != 1.0
    @values[@idx] = v
    @dthash[dt] = @idx
    @idx += 1
  end 
  
  def finalize
    # perhaps we should force a gc run here.
  end
  
  # getters 
  
  def size
    return @idx
  end
  
  def length 
    return @idx
  end
  
  # return index at Datetime (hash key]
  def index_for_date(dt) 
    return @dthash[dt]
  end
  
  # return value at dt
  def value_at_date(dt)
    return @values[@dthash[dt]]
  end
   
  # return value at idx (integer)
  def value_at_index(idx)
    return @values[idx]
  end
  
  def [] (idx)
    return @values[idx]
  end
  
  def name
     return @short_name
  end
  
  def bool?
    return @bool
  end
  
  def start_date
    return @startpt
  end
  
  def end_date
    return @endpt
  end 
  
  def minv
    return @minv
  end
  
  def maxv
    return @maxv
  end
  
  def vrange 
    return [@minv, @maxv]
  end
  
end
