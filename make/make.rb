module Make
  include FileUtils

  def copy_files_to_dist
    if ENV['APP']
      if APP['clone']
        sh APP['clone'].gsub(/^git /, "#{GIT} --git-dir=#{ENV['APP']}/.git ")
      else
        cp_r ENV['APP'], "dist/app"
      end
      if APP['ignore']
        APP['ignore'].each do |nn|
          rm_rf "dist/app/#{nn}"
        end
      end
    end

    cp_r  "fonts", "dist/fonts"
    cp_r  "lib", "dist/lib"
    cp_r  "samples", "dist/samples"
    cp_r  "static", "dist/static"
    cp    "README", "dist/README.txt"
    cp    "CHANGELOG", "dist/CHANGELOG.txt"
    cp    "COPYING", "dist/COPYING.txt"
  end
end
