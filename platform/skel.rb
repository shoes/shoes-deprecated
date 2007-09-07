def skel_replace(line)
  line.gsub! /\s+%DEFAULTS%/ do
    if APPARGS
      args = APPARGS.split(/\s+/)
      %{
        char *default_argv[] = {argv[0], #{args.inspect[1..-2]}};
        argv = default_argv;
        argc = #{args.length + 1};
      }
    end
  end
  line
end

# preprocess .skel
task :build_skel do |t|
  Dir["bin/*.skel"].each do |src|
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
