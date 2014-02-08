# Deal with Encoding issues in versions of Shoes and platforms
#
case RUBY_PLATFORM
  when /mingw/
    IO.read(File.join DIR, 'encoding.data').each_line do |fn| 
      $glbstr << "Add: #{fn}"
      require File.join(DIR, "ruby/lib/#{RUBY_PLATFORM}/enc", fn.chomp)
    end
  when /linux/
    progress_str = "Fixing encodings for Linux"
    #$stderr.puts progress_str
  when /darwin/ 
    progress_str = "Fixing encodings for OSX"
    #$stderr.puts progress_str
end
