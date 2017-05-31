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
    mkdir_p "dist/lib/shoes"
    Dir.chdir "dist/lib/shoes" do
      Dir["../../../lib/shoes/*.rb"].each do |f|
        #puts "SymLinking #{f}"
        ln_s f, "." unless File.symlink? File.basename(f)
      end
    end
    Dir.chdir "dist/lib" do
      ln_s "../../lib/shoes.rb" , "shoes.rb" unless File.symlink? "shoes.rb"
      # link to exerb
      ln_s "../../lib/exerb", "exerb" unless File.symlink? "exerb"
    end
    cp_r  "fonts", "dist/fonts"
    cp_r  "samples", "dist/samples"
    Dir.chdir "dist" do
      ln_s  "../static",  "." unless File.symlink? 'static'
    end

    cp    "README.md", "dist/README.txt"
    cp    "CHANGELOG", "dist/CHANGELOG.txt"
    cp    "COPYING", "dist/COPYING.txt"
    touch 'zzsetup.done'
  end
end
