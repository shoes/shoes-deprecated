# this is make _like_. very hand crafted. Modify with care and wisdome.
Base_h = FileList["shoes/*.h"] - ["shoes/appwin32.h", "shoes/version.h"]
# touching one of those could/should rebuild everything. 
#
# Keep the file paths to .o files in Constant OBJS - not the
# same as OBJ in the older build. NOT THE SAME. May not be used everywhere
# or anywhere 
OBJS = []

# Shoes shoes/base.lib (canvas, ruby, image....
tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
mkdir_p tp
base_src = FileList["shoes/*.c"]
base_obj = []
base_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/#{fnm}.o"
  base_obj  << o
  #$stderr.puts "create base task for #{o} => #{c}"
  file o => [c] + Base_h do |t|
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
#puts "base_obj = #{base_obj}"
file "#{tp}/base.lib" => base_obj do
  objs = Dir["#{tp}/*.o"]
  OBJS.concat objs
  sh "ar -rc #{tp}/base.lib #{objs.join(' ')}"
end

# Shoes/widget ruby interface (aka types/)
mkdir_p "#{tp}/types"
rbwidget_src = FileList["shoes/types/*.c"]
rbwidget_obj = []
rbwidget_hdr = []
rbwidget_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/types/#{fnm}.o"
  rbwidget_obj << o
  h = c.gsub(/.c$/, '.h')
  rbwidget_hdr << h
  file o => [c] + [h] + Base_h do |t| 
   sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/types/widgets.lib" => rbwidget_obj do
  objs = Dir["#{tp}/types/*.o"]
  OBJS.concat objs
  sh "ar -rc #{tp}/types/widgets.lib #{objs.join(' ')}"
end

# Shoe Native
nat_src = []
nat_obj = []
mkdir_p "#{tp}/native"
if RUBY_PLATFORM =~ /darwin/
  file "#{tp}/native/cocoa.o" => ["shoes/native/cocoa.m", "shoes/native/cocoa.h"] +
      Base_h
  file "#{tp}/native/native/lib" => ["shoes/native/cocoa.o"] do
    sh "ar -rc #{tp}/native/native.lib shoes/native/cocoa.o"
  end
else
  nat_src = FileList['shoes/native/gtk/*.c']
  nat_src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/native/#{fnm}.o"
    #o = c.gsub(/.c$/, '.o')
    nat_obj << o
    h = c.gsub(/.c$/, '.h')
    file o => [c] + [h] + ['shoes/native/native.h'] + Base_h do
      sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/native/gtk.o" => ["shoes/native/gtk.c", "shoes/native/gtk.h", "shoes/native/native.h"] + Base_h do
    sh "#{CC} -o #{tp}/native/gtk.o -I. -c #{LINUX_CFLAGS} shoes/native/gtk.c"
  end
  file "#{tp}/native/native.lib" => ["#{tp}/native/gtk.o"] + nat_obj do
    OBJS.concat nat_obj
    OBJS.concat ["#{tp}/native/gtk.o"] 
    sh "ar -rc #{tp}/native/native.lib #{tp}/native/gtk.o #{nat_obj.join(' ')}"
  end
end

# Shoes/http
dnl_src = []
dnl_obj = []
mkdir_p "#{tp}/http"
if RUBY_PLATFORM =~ /darwin/
  dnl_src = ["shoes/http/rbload.c", "nsurl.m"]
else
  dnl_src = ["shoes/http/rbload.c"]
end
dnl_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/http/#{fnm}.o"
  #o = c.gsub(/(.c|.m)$/, '.o')
  dnl_obj << o
  file o => [c] + Base_h do 
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/http/download.lib" => dnl_obj do
  OBJS.concat dnl_obj
  sh "ar -rc #{tp}/http/download.lib #{dnl_obj.join(' ')}"
end

# Plot
mkdir_p "#{tp}/plot"
plot_src = FileList['shoes/plot/*.c']
plot_obj = []
plot_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/plot/#{fnm}.o"
  #o = c.gsub(/.c$/, '.o')
  plot_obj << o
  file o => [c] + ["shoes/plot/plot.h", "shoes/plot/plot_util.c", "shoes/plot/chart_series.c"] do
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end

file "#{tp}/plot/plot.lib" => plot_obj do 
  objs = Dir['shoes/plot/*.o']
  OBJS.concat objs
  sh "ar -rc #{tp}/plot/plot.lib #{objs.join(' ')}"
end

# Console 
mkdir_p "#{tp}/console"
if RUBY_PLATFORM =~ /darwin/
  src = ["shoes/console/tesi.c", "shoes/console/colortab.c", "shoes/console/cocoa-term.c"]
  obj =[]
  src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/plot/#{fnm}.o"
    #o = c.gsub(/(.c|.m)$/, '.o')
    obj << o
    file o => [c] + ["shoes/console/tesi.h"] do
      sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{TGT_DIR}/tmp/shoes/console/console.lib" =>  obj do
    OBJS.concat obj
    sh "ar -rc s#{TGT_DIR}/tmp/hoes/console/console.lib #{obj.join(' ')}"
  end
else 
  src = ["shoes/console/tesi.c", "shoes/console/colortab.c", "shoes/console/gtk-terminal.c"]
  obj = []
  src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/plot/#{fnm}.o"
    #o = c.gsub(/.c$/, '.o')
    obj << o
    file o => [c] + ["shoes/console/tesi.h"] do
        sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/console/console.lib" =>  obj do
    OBJS.concat obj
    sh "ar -rc #{tp}/console/console.lib #{obj.join(' ')}"
  end
end




