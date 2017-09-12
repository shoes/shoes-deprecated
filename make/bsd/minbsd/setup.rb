module Make
  include FileUtils


  # Set up symlinks to lib/shoes and lib/shoes.rb so that they
  # can be edited and tested without a rake clean/build every time we 
  # change a lib/shoes/*.rb  
  # They'll be copied (not linked) when rake install occurs. Be very
  # careful. Only Link to FILES, not to directories. Fileutils.ln_s may
  # not be the same as linux ln -s. 
  def static_setup (solocs)
    srcloc= `pwd`.strip
    puts "setup: #{srcloc}"
=begin  
    cp_r  "fonts", "#{TGT_DIR}/fonts"
    mkdir_p "#{TGT_DIR}/lib/shoes"
    Dir.chdir "#{TGT_DIR}/lib/shoes" do
      Dir["#{srcloc}/lib/shoes/*.rb"].each do |f|
        #puts "SymLinking #{f}"
        ln_s f, "." unless File.symlink? File.basename(f)
      end
    end
    Dir.chdir "#{TGT_DIR}/lib" do
      ln_s "#{srcloc}/lib/shoes.rb" , "shoes.rb" unless File.symlink? "shoes.rb"
      # link to exerb
      ln_s "#{srcloc}/lib/exerb", "exerb" unless File.symlink? "exerb"
    end
  
    #cp_r  "samples", "#{TGT_DIR}/samples"
    mkdir_p "#{TGT_DIR}/samples"
    ['simple', 'good', 'expert'].each do |d|
      mkdir_p "#{TGT_DIR}/samples/#{d}"
      Dir.chdir "#{TGT_DIR}/samples/#{d}" do
        Dir["../../../samples/#{d}/*"].each do |f|
          ln_s f, '.' unless File.symlink? File.basename(f)
        end
      end
    end
=end
    ln_s "#{srcloc}/lib", TGT_DIR
    ln_s "#{srcloc}/samples", TGT_DIR
    ln_s "#{srcloc}/static",  TGT_DIR
    ln_s "#{srcloc}/fonts", TGT_DIR

    cp    "README.md", TGT_DIR
    cp    "CHANGELOG", TGT_DIR
    cp    "COPYING", TGT_DIR
  end
end
