#find resources under profiling.
Shoes.app do
 button "where am I" do
   para "curr_dir is #{Dir.getwd}"
   found  =  File.exist? 'vivaldi.ttf' 
   para "Found resourse #{found}"
 end
end
