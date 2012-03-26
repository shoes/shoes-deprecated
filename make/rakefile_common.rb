task :build_os => [:buildenv_linux, :build_skel, "dist/#{NAME}"]

task :buildenv_linux do
  unless ENV['VIDEO']
    rm_rf "dist"
    mkdir_p "dist"
  end
end
