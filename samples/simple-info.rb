Shoes.app height: 800, width: 800 do
  para "Startup info\n"
  para "Ruby Version: #{RUBY_VERSION} on #{RUBY_PLATFORM}\n"
  para "Shoes Release: #{Shoes::RELEASE_NAME}\n"
  para "    built on #{Shoes::RELEASE_BUILD_DATE}\n"
  para "    Fit: #{Shoes::RELEASE_TYPE}\n"
  para "Gems Version #{Gem::RubyGemsVersion}"
  if defined?(ShoesGemJailBreak)
   para "Jailbreak == #{ShoesGemJailBreak}"
  else
   para "Cannot determine JailBreak status?"
  end
  stack do
      para "Env Variables"
      @env = edit_box height: 150, width: 0.95, margin: 8
	  ct = ["ENV vars:\n"]
	  ENV.each_key do |i|
	    ct <<  "#{i}=#{ENV[i]}\n"
	  end
	  @env.text = ct.join
	  
	  ct = ["$: Array of ext/gem dirs:\n"]
	  para ct[0].rstrip
	  @edir = edit_box height: 150, width: 0.95, margin: 8
	  $:.each do |gdir|
	    ct << "#{gdir}\n"
	  end
	  @edir.text = ct.join
	  
	  ct = ["RbConfig::CONFIG\n"]
	  para ct[0].strip
	  @rbc = edit_box height: 150, width: 0.95, margin: 8
	  RbConfig::CONFIG.each_key do |j|
	     ct << "'#{j}' = #{RbConfig::CONFIG[j]}\n"
	  end
	  @rbc.text = ct.join
	  
	  hsh = {}
	  ct =  ["$\" before require\n"]
	  para ct[0].rstrip
	  @ld1 = edit_box height: 150, width: 0.95, margin: 8
	  $".each do |k| 
	    ct << "#{k}\n"
	    hsh[k] = true
	  end
	  @ld1.text = ct.join
	  
	  ct = ["$\" additions after require\n"]
	  para ct[0].rstrip
	  @ld2 = edit_box height: 150, width: 0.95, margin: 8
      require 'bigdecimal'
	  #require 'hpricot'
      if ShoesGemJailBreak
	    #require 'fpm'
        #require 'nokogiri'
      end
	  $".each do |f|
	    ct << "#{f}\n" unless hsh[f]
	  end
	  @ld2.text =ct.join
	  flow do
	    button "Save to file..." do
	      fn = ask_save_file
	      if fn && f=File.open(fn,"w")
	         f.puts @env.text
	         f.puts ("\n")
	         f.puts @edir.text
	         f.puts ("\n")
	         f.puts @rbc.text
	         f.puts ("\n")
	         f.puts @ld1.text
	         f.puts ("\n")
	         f.puts @ld2.text
	         f.close
	      end
	    end
	  end
  end
end
