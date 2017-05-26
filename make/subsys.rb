# this is make _like_. very detailed.
Base_h = FileList["shoes/*.h"] - ["shoes/appwin32.h", "shoes/version.h"]
# touching one of those chould rebuild everything. 

# Shoes shoes/base.lib (canvas, ruby, image....
base_src = FileList["shoes/*.c"]
base_obj = []
base_src.each do |c|
  o = c.gsub(/.c$/, '.o')
  base_obj  << o
  file o => [c] + Base_h
end

file "shoes/base.lib" => base_obj do
  objs = Dir['shoes/*.o']
  sh "ar -rc shoes/base.lib #{objs.join(' ')}"
  sh "ranlib shoes/base.lib"
end

# Shoes/widget ruby interface (aka types/)
rbwidget_src = FileList["shoes/types/*.c"]
rbwidget_obj = []
rbwidget_hdr = []
rbwidget_src.each do |c|
  o = c.gsub(/.c$/, '.o')
  rbwidget_obj << o
  h = c.gsub(/.c$/, '.h')
  rbwidget_hdr << h
  file o => [c] + [h] + Base_h
end

file "shoes/types/widgets.lib" => rbwidget_obj do
  objs = Dir['shoes/types/*.o']
  sh "ar -rc shoes/types/widgets.lib #{objs.join(' ')}"
  sh "ranlib shoes/types/widgets.lib"
end

# Shoe Native
nat_src = []
nat_obj = []
if RUBY_PLATFORM =~ /darwin/
  #TODO ? 
  file "shoes/native/cocoa.o" => ["shoes/native/cocoa.m", "shoes/native/cocoa.h"] +
      Base_h do
    sh "ar -rcs shoes/native/native.lib shoes/native/cocoa.o"
  end
else
  nat_src = FileList['shoes/native/gtk/*.c']
  nat_src.each do |c|
    o = c.gsub(/.c$/, '.o')
    nat_obj << o
    h = c.gsub(/.c$/, '.h')
    file o => [c] + [h] + ['shoes/native/native.h'] + Base_h
  end
  file "shoes/native/gtk.o" => ["shoes/native/gtk.h", "shoes/native/native.h"] + Base_h
  file "shoes/native/native.lib" => ['shoes/native/gtk.o'] + nat_obj do
    sh "ar -rc shoes/native/native.lib shoes/native/gtk.o #{nat_obj.join(' ')}"
    sh "ranlib shoes/native/native.lib"
  end
end



# Shoes/http
dnl_src = []
dnl_obj = []
if RUBY_PLATFORM =~ /darwin/
  #TODO - compile nsurl.m and rbload.c?
else
  dnl_src = ["shoes/http/rbload.c"]
  dnl_obj = []
end
dnl_src.each do |c|
  o = c.gsub(/.c$/, '.o')
  dnl_obj << o
  file o => [c] + Base_h
end
file "shoes/http/download.lib" => dnl_obj do
  sh "ar -rc shoes/http/download.lib #{dnl_obj.join(' ')}"
  sh "ranlib shoes/http/download.lib"
end

# Plot
Plot_Src = FileList['shoes/plot/*.c']
#$stderr.puts "plot/*.c: #{Plot_Src}"
Plot_Obj = []
Plot_Src.each do |c|
  o = c.gsub(/.c$/, '.o')
  #$stderr.puts "creating file task #{o} => #{[c]}"
  Plot_Obj << o
  file o => [c] + ["shoes/plot/plot.h", "shoes/plot/plot_util.c", "shoes/plot/chart_series.c"]
end

file "shoes/plot/plot.lib" => Plot_Obj do 
  objs = Dir['shoes/plot/*.o']
  #$stderr.puts "objs: #{objs.inspect}"
  sh "ar -rc shoes/plot/plot.lib #{objs.join(' ')}"
  sh "ranlib shoes/plot/plot.lib"
end

# Console 
if RUBY_PLATFORM =~ /darwin/
  # TODO 
  file "shoes/console/console.lib" => ["shoes/console/tesi.o", "shoes/console/cocoa-term.h",
    "shoes/console/cocoa-term.m"] do
    # some sort of osx ar -rc
  end
else 
  src = ["shoes/console/tesi.c", "shoes/console/colortab.c", "shoes/console/gtk-terminal.c"]
  obj = []
  src.each do |c|
    o = c.gsub(/.c$/, '.o')
    obj << o
    file o => [c] + ["shoes/console/tesi.h"]
  end
  file "shoes/console/console.lib" =>  obj do
    sh "ar -rc shoes/console/console.lib #{obj.join(' ')}"
    sh "ranlib shoes/console/console.lib"
  end
end


