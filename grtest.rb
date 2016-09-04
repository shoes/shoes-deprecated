require 'csv'


def load_test (filename, vals, obvs)
  hdr_name = ''
  CSV.foreach(filename, 'r') do |row|
      hdr_name = row[0]
      dts = row[1].to_str
      #y = dts[0..3].to_i
      #m = dts[4..5].to_i
      #d = dts[6..7].to_i
      #dt = DateTime.new(y,m,d,16,0,0,'-4')
      v = row[2].to_f
      obvs << "#{dts[2..3]}-#{dts[4..5]}-#{dts[6..7]}"
      vals << v
  end
  return hdr_name
end

Shoes.app width: 620, height: 610 do
  @vals = []
  @obvs = []
  @name = ''
  zooming = false;
  zoom_beg = 0
  zoom_end = 0;
  stack do
    flow do 
      button "quit" do Shoes.quit end
      button "load csv..." do 
        #short_name = load_test "/home/ccoupe/Projects/JModel-1.4/ruby/tstest.csv",  @vals, @obvs
        filename = ask_open_file
        if filename 
          short_name = load_test filename, @vals, @obvs
          para "loaded #{@vals.size}"
          @grf.add  num_obs: @vals.size, values: @vals, maxv: @vals.max * 1.01,
              minv: @vals.min, name: short_name, xobs: @obvs, nubs: :true
          zoom_end = @vals.size
        end
      end
      button "in %25" do
       range = zoom_end - zoom_beg
       zoom_beg = (zoom_beg + (range * 0.125)).to_i
       zoom_end = (zoom_end - (range * 0.125)).to_i
       @grf.zoom zoom_beg, zoom_end
      end
      button "out %25" do
        range = zoom_end - zoom_beg
        zoom_beg = (zoom_beg - (range * 0.125)).to_i
        zoom_end = (zoom_end + (range * 0.125)).to_i
        @grf.zoom zoom_beg, zoom_end
      end
      button "reset" do
        zoom_beg = 0
        zoom_end = @vals.size
        @grf.zoom zoom_beg, zoom_end
      end
    end
    @grf = plot 600, 400,  title: "Test - ^SPX ", caption: "1993 Jan 4 to May 25",
      x_ticks: 8, y_ticks: 10,  auto_grid: true
  end
end

