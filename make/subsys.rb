# this is make _like_. very detailed.
Base_h = FileList["shoes/*.h"] - ["shoes/appwin32.h", "shoes/version.h"]
# touching one of those should rebuild everything. 

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
end

# Shoes/types - 

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
  end
end

