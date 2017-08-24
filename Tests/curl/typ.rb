require 'typhoeus'
Shoes.app do
  button "typheous" do
    st_time = Time.now
    filesz = 0
    downloaded_file = File.open 'huge.iso', 'wb'
    @th = Thread.new do
      request = Typhoeus::Request.new("http://walkabout.mvmanila.com/public/share/Ytm-2.exe")
      request.on_headers do |response|
        if response.code != 200
          raise "Request failed"
        end
        puts "length #{response.headers['Content-Length']}"
        filesz = response.headers['Content-Length'].to_i
      end
      request.on_body do |chunk|
        $stdout.puts "."
        downloaded_file.write(chunk)
      end
      request.on_complete do |response|
        downloaded_file.close
        end_time = Time.now
        elapsed = end_time.to_i - st_time.to_i
        $stderr.puts "Finished in #{elapsed} secs #{(filesz/1024)/elapsed} KB/s"
        @th.join
      end
      request.run
    end
  end
end
