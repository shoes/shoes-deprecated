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
    cp    "README.md", "dist/README.txt"
    cp    "CHANGELOG", "dist/CHANGELOG.txt"
    cp    "COPYING", "dist/COPYING.txt"
  end

  def cc(t)
    sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
  end

  # Subs in special variables
  def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
    File.open(after, 'w') do |a|
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(reg) do
            if reg2.include? '\1'
              reg2.gsub(%r!\\1!, Object.const_get($1))
            else
              reg2
            end
          end
        end
      end
    end
  end

  def copy_files glob, dir
    FileList[glob].each { |f| cp_r f, dir }
  end

  # copy ruby libs to dist so we can link relative
  def pre_build
    $stderr.puts "Pre_build #{EXT_RUBY}"
    mkdir_p "dist/ruby"
    cp "#{::EXT_RUBY}/lib/lib#{::RUBY_SO}.so", "dist/lib#{::RUBY_SO}.so"
  end

  def common_build
    mkdir_p "dist/ruby"
    cp_r  "#{EXT_RUBY}/lib/ruby/#{RUBY_V}", "dist/ruby/lib"
    unless ENV['STANDARD']
      %w[soap wsdl xsd].each do |libn|
        rm_rf "dist/ruby/lib/#{libn}"
      end
    end
    %w[req/rake/lib/*].each do |rdir|
      FileList[rdir].each { |rlib| cp_r rlib, "dist/ruby/lib" }
    end
    %w[req/binject/ext/binject_c req/bloopsaphone/ext/bloops req/chipmunk/ext/chipmunk].
      each { |xdir| copy_ext xdir, "dist/ruby/lib/#{SHOES_RUBY_ARCH}" }

    gdir = "dist/ruby/gems/#{RUBY_V}"
    #{}.each do |gemn, xdir|
    {'sqlite3' => 'lib'}.each do |gemn, xdir|
      spec = eval(File.read("req/#{gemn}/gemspec"))
      mkdir_p "#{gdir}/specifications"
      mkdir_p "#{gdir}/gems/#{spec.full_name}/lib"
      FileList["req/#{gemn}/lib/*"].each { |rlib| cp_r rlib, "#{gdir}/gems/#{spec.full_name}/lib" }
      mkdir_p "#{gdir}/gems/#{spec.full_name}/#{xdir}"
      FileList["req/#{gemn}/ext/*"].each { |elib| copy_ext elib, "#{gdir}/gems/#{spec.full_name}/#{xdir}" }
      cp "req/#{gemn}/gemspec", "#{gdir}/specifications/#{spec.full_name}.gemspec"
    end
  end

  # Check the environment
  def env(x)
    unless ENV[x]
      abort "Your #{x} environment variable is not set!"
    end
    ENV[x]
  end
end
