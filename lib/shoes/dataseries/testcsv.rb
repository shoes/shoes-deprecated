require 'csv'
Class TestCsv 

  def initialize(file)
    ary = []
    idx = 0
    short = ''
    CSV.foreach(file) do |row|
      short = row[0]
      dt = Datetime(row[1])
      v = row[2]
    end
  end
end
