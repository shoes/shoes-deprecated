module CsvSeries
  require "lib/shoes/dataseries/timeseries.rb"
  require 'date'
  require 'csv'

  def CsvSeries.create(file)
    ds = TimeSeries.new(File.basename(file, '.csv'))
    CSV.foreach(file, 'r') do |row|
      short = row[0]
      dts = row[1]
      y = dts[0..3].to_i
      m = dts[4..5].to_i
      d = dts[6..7].to_i
      dt = DateTime.new(y,m,d,16,0,0,'-4')
      v = row[2].to_f
      ds.append(dt, v)
    end
    ds.finalize() # good practice - it might do something useful 
    return ds
  end
  
end #module
