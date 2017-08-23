Shoes.app do
  background "#eee"
  @list = stack do
    para "Enter a URL to download:", :margin => [10, 8, 10, 0]
    flow :margin => 10 do
      @url = edit_line :width => -120
      #@url.text ='http://walkabout.mvmanila.com/public/share/test-csv.tgz'
      @url.text ='http://walkabout.mvmanila.com/public/share/Ytm-2.exe'
      st_time = 0
      end_time = 0
      totalsz = 0
      button "Download", :width => 120 do
        st_time = Time.now
        @list.append do
          stack do
            background "#eee".."#ccd"
            stack :margin => 10 do
              dld = nil
              para @url.text, " [", link("cancel") { dld.abort }, "]", :margin => 0
              d = inscription "Beginning transfer.", :margin => 0
              dld = download @url.text, :save => File.basename(@url.text), 
                :finish => proc { |dl|
                     end_time = Time.now
                     secs = (end_time.to_i - st_time.to_i)
                     kb = dl.length < 1024 ? 1: dl.length / 1024
                     d.text = "Download completed KB/s: #{kb/secs} secs: #{secs}"
                  }
            end
          end
        end
      end
    end
  end
end
