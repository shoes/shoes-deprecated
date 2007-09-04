def skel_replace(line)
  line.gsub! /\s+%DEFAULTS\((\w+)\)%/ do
    if APPARGS
      %{
        int #$1 = TRUE;
        char *default_argv[] = {argv[0], #{APPARGS.inspect[1..-2]}};
        argv = default_argv;
        argc = #{APPARGS.length + 1};
      }
    end
  end
  line
end

# preprocess .skel
task :build_skel do |t|
  Dir["**/*.skel"].each do |src|
    name = src.gsub(/\.skel$/, '.c')
    File.open(src) do |skel|
      File.open(name, 'w') do |c|
        skel.each_line do |line|
          c << skel_replace(line)
        end
      end
    end
  end
end
