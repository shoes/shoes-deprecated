Shoes.app do
  require 'shoes/data'
  require 'shoes/image'
  stack do
    flow do
      @el = edit_line "#{DIR}/static/shoes-icon-walkabout.png"
      #@el = edit_line "https://shoes.mvmanila.com/public/images/dino.jpg"
      @cb = check; para "Cached?"
      button "(Re)load" do
        @img.clear
        @img.append do
          image @el.text, cache: @cb.checked
        end
      end
      button "Show cache" do
        @cview.clear
        @cview.append do
          eb = edit_box width: 400
          DATABASE.each do |key, value|
            eb.append "#{key} -> #{value}"
          end
        end
      end
      button "clear caches" do
        DATABASE.each do |k, val|
          v = val.split('|')
          path = Shoes::image_cache_path v[1], File.extname(k)
          puts "delete: #{path}"
          File.delete path if File.exist? path
        end        
        DATABASE.clear
        DATABASE.close
        quit if confirm "Please restart Shoes"
      end
      para "Global cache: " 
      @sw = switch width: 80 do |n|
        app.cache = n.active?
        @cb.checked = n.active?
      end
    end
    @img = flow {} 
    @cview = flow {}
  end
  start do
    @sw.active = app.cache
    @cb.checked = app.cache
  end
end
