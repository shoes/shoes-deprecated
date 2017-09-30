Shoes.app do
  background "#eee"
  @list = stack do
    @img = image '../static/man-builds.png'
    para "Enter a image URL to download:", :margin => [10, 8, 10, 0]
    flow :margin => 10 do
      @url = edit_line :width => -120
      @url.text ='http://shoes.mvmanila.com/public/images/dino.jpg'
      button "Download", :width => 120 do
        @list.append do
          stack do
            background "#eee".."#ccd"
            stack :margin => 10 do
               image  @url.text
            end
          end
        end
      end
    end
  end
end
