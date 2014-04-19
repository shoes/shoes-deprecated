Shoes.app do
  background "#eee"
  @list = stack do
    para "Enter a URL to download:", :margin => [10, 8, 10, 0]
    flow :margin => 10 do
      @url = edit_line :width => -120
      @url.text ='http://shoes.mvmanila.com'
      button "Download", :width => 120 do
        @list.append do
          stack do
            background "#eee".."#ccd"
            stack :margin => 10 do
              dld = nil
              download @url.text do |r| 
                puts "in block"
                para "Status: #{r.response.status}\nHeaders:"
                r.response.headers.each {|k,v| para "#{k.strip}: #{v.strip}"}
                sz = r.response.body.size
                para "Body size: #{sz}\n"
                if sz > 0
                  lines = r.response.body.split
                  para "First: #{lines[0]}\n"
                  para "Last:  #{lines[-1]}\n"
                end
              end
            end
          end
        end
      end
    end
  end
end
