module Make
  include FileUtils


  # Set up symlinks to lib/shoes and lib/shoes.rb so that they
  # can be edited and tested without a rake clean/build every time we 
  # change a lib/shoes/*.rb  
  # They'll be copied (not linked) when rake install occurs. Be very
  # careful. Only Link to FILES, not to directories. Fileutils.ln_s may
  # not be the same as linux ln -s. 
  def static_setup (solocs)
    puts "setup: #{pwd}"
    mkdir_p "#{TGT_DIR}/lib/shoes"
    Dir.chdir "#{TGT_DIR}/lib/shoes" do
      Dir["../../../lib/shoes/*.rb"].each do |f|
        #puts "SymLinking #{f}"
        ln_s f, "." unless File.symlink? File.basename(f)
      end
    end
    Dir.chdir "#{TGT_DIR}/lib" do
      ln_s "../../lib/shoes.rb" , "shoes.rb" unless File.symlink? "shoes.rb"
      # link to exerb
      ln_s "../../lib/exerb", "exerb" unless File.symlink? "exerb"
    end
    cp_r  "fonts", "#{TGT_DIR}/fonts"
    
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
    Dir.chdir "#{TGT_DIR}" do
      ln_s  "../static",  "." unless File.symlink? 'static'
    end

    cp    "README.md", "#{TGT_DIR}/README.txt"
    cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
  end
end
