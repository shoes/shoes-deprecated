require 'csv'
# csv can be nasty with frozen values. Grrr.

class DataSeries

  @name = ""
  @values = []
  @obs = []
  attr_accessor :name, :values, :obvs

  def initialize(filename)
    @values = []
    @obvs = []
    load_file(filename)
  end

  def load_csv (filename, vals, obvs)
    hdr_name = nil
    CSV.foreach(filename, 'r') do |row|
      if !hdr_name
        hdr_name = row[0].dup if !hdr_name
      end
      dts = row[1].to_str.dup
      #y = dts[0..3].to_i
      #m = dts[4..5].to_i
      #d = dts[6..7].to_i
      #dt = DateTime.new(y,m,d,16,0,0,'-4')
      v = row[2].to_f
      @obvs <<  "#{dts[2..3]}-#{dts[4..5]}-#{dts[6..7]}"
      @values << v
    end
    return hdr_name
  end

  # I know the data is well formed csv
  def load_file (filename)
    @name = nil
    File.open(filename, mode: 'r').each do |ln|
      flds = ln.split(',')
      if !@name
        @name = flds[0]
      end
      dts = flds[1];
      @obvs <<  "#{dts[2..3]}-#{dts[4..5]}-#{dts[6..7]}"
      @values << flds[2].to_f
    end
    #puts "load_file: #{@values.size}"
  end
  
  def size 
    return @values.size
  end
  
  def max
    @values.max
  end
  
  def min
   @values.min
  end
  
  def name
   @name
  end
  
end #class 

Shoes.app width: 620, height: 610 do
  @series =[] 
  @zooming = false;
  @zoom_beg = 0
  @zoom_end = 0;
  
  def zoom_in
    range = @zoom_end - @zoom_beg
    @zoom_beg = (@zoom_beg + (range * 0.125)).to_i
    @zoom_end = (@zoom_end - (range * 0.125)).to_i
    puts "zoom in #{@zoom_beg} #{@zoom_end}"
    @zooming = true;
    @grf.zoom @zoom_beg, @zoom_end
  end
  
  def zoom_out
    range = @zoom_end - @zoom_beg
    sz = @series[0].size
    @zoom_beg = (@zoom_beg - (range * 0.125)).to_i
    @zoom_beg = 0 if @zoom_beg < 0
    @zoom_end = (@zoom_end + (range * 0.125)).to_i
    @zoom_end = sz if @zoom_end > sz
    puts "zoom out #{@zoom_beg} #{@zoom_end}"
    @grf.zoom @zoom_beg, @zoom_end
    if @zoom_beg == 0 && @zoom_end == sz
      puts "zoom off"
      @zooming = false
    end
  end
  
  def zoom_scroll_left
    if @zooming
     range = @zoom_end - @zoom_beg
     sz = @series[0].size
     @zoom_beg = (@zoom_beg - (range * 0.125)).to_i
     @zoom_beg = 0 if @zoom_beg < 0
     @zoom_end = (@zoom_end - (range * 0.125)).to_i
     @zoom_end = sz if @zoom_end > sz
     puts "scroll left #{@zoom_beg} #{@zoom_end}"
     @grf.zoom @zoom_beg, @zoom_end
    end
  end
  
  def zoom_scroll_right
    if @zooming
      range = @zoom_end - @zoom_beg
      sz = @series[0].size
      @zoom_beg = (@zoom_beg + (range * 0.125)).to_i
      @zoom_beg = 0 if @zoom_beg < 0
      @zoom_end = (@zoom_end + (range * 0.125)).to_i
      @zoom_end = sz if @zoom_end > sz
      puts "scroll right #{@zoom_beg} #{@zoom_end}"
      @grf.zoom @zoom_beg, @zoom_end
    end
  end
  
  def zoom_end_key
    @zoom_beg = 0
    @zoom_end = @series[0].size
    @zooming = false
    @grf.zoom @zoom_beg, @zoom_end
  end
  
  def zoom_home_key
    @zoom_beg = 0
    @zoom_end = @series[0].size
    @zooming = false
    @grf.zoom @zoom_beg, @zoom_end
  end
  
  def max (a, b)
    return (a >= b) ? a : b
  end
  
  def min (a, b)
    return (a <= b) ? a : b
  end
  
  # like zoom in, but it centers the display on the clicked observation (index)
  # Note that the idx lies between displayed beg and end indices already
  def zoom_center(x)
    idx = @grf.near_x x
    printf("zoom center start: %3d % 4d % 4d % 4d\n", x, @zoom_beg, idx, @zoom_end)
    range = @zoom_end - @zoom_beg
    cpidx = idx / range
  
    newl = (@zoom_beg + (range * 0.125)).to_i
    newr = (@zoom_end - (range * 0.125)).to_i
    @zoom_beg = newl
    @zoom_end = newr
    printf("zoom center end:   %3d % 4d % 4d % 4d \n", x, @zoom_beg, idx, @zoom_end)
    @zooming = true;
    @grf.zoom @zoom_beg, @zoom_end
  end
  
  stack do
    flow do 
      button "quit" do Shoes.quit end
      button "load csv..." do 
        Dir.chdir("/home/ccoupe/Projects/JModel-1.4/Data/93-13/") do 
          filename = ask_open_file
          if filename 
              @series << DataSeries.new(filename)
              newidx = @series.size - 1
              #puts "newidx = #{newidx}"
              ser = @series[newidx]
              @grf.add  num_obs: ser.size, values: ser.values, maxv: ser.max * 1.01,
                minv: ser.min, name: ser.name, xobs: ser.obvs, nubs: :true
              @zoom_end = ser.size
          end
        end
      end
      button "in %25" do
        zoom_in
      end
      button "out %25" do
        zoom_out
      end
      button "reset" do
        zoom_home_key
      end
      button "save as" do
        file = ask_save_file
        @grf.save_as file if file
      end
    end
    @grf = plot 600, 400,  title: "Explore Market Data", caption: "depends on the data. eh?",
      x_ticks: 8, y_ticks: 10,  auto_grid: true, click: proc {|btn, l, t| zoom_center l}, chart: "timeseries"
    keypress do |k|
      #puts "key: #{k.inspect}"
      case k
        when :home  
          zoom_home_key
        when :end
          zoom_end_key
        when "+" 
          zoom_in
        when "-" , "_"
          zoom_out
        when :left
          zoom_scroll_left
        when :right
          zoom_scroll_right
      end
    end
  end
end
