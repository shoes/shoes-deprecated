# test csv loading
require_relative "lib/shoes/dataseries/dataseries.rb"
#puts "loaded testcsv.rb"
require 'date'
require 'csv'

class TestCsv 
  def initialize(file)
    dary = []
    vary = []
    hsh = {}
    idx = 0
    maxv = 0.0
    minv = 1000000.0
    short = ''
    boolseries = false
    CSV.foreach(file, 'r') do |row|
      short = row[0]
      dts = row[1]
      y = dts[0..3].to_i
      m = dts[4..5].to_i
      d = dts[6..7].to_i
      dt = DateTime.new(y,m,d,16,0,0,'-4')
      v = row[2].to_f
      maxv = v if v > maxv
      minv = v if v < minv
      dary[idx] = dt
      vary[idx] = v
      hsh[dt] = v
      idx += 1
    end
    begpt = dary[0]
    endpt = dary[idx-1]
    if maxv == 1.0 && minv == 0.0
       boolseries = true
    end
    ds = DataSeries.new(short, boolseries, dary, hsh, begpt, endpt, minv, maxv)
    puts "have  #{ds.short_name} type: #{ds.type} vrange #{ds.minv}:#{ds.maxv}"
    puts "start #{ds.values[ds.startpt]} #{ds.startpt} "
    puts "end   #{ds.values[ds.endpt]} #{ds.endpt}"
    rdidx = rand(ds.values.length);
    rkey = ds.ary[rdidx]
    puts "random: #{rdidx}: is #{rkey} value: #{ds.values[rkey]}"
    return ds
  end
end


spx = TestCsv.new('^SPX')
