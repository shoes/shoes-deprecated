Shoes.app do
  require 'shoes/data'
  require 'shoes/image'
  stack do
    flow do
      #@el = edit_line "#{DIR}/static/shoes-icon-walkabout.png"
      @el = edit_line "https://shoes.mvmanila.com/public/images/dino.jpg"
      @cb = check; para "Don't cache"
      button "(Re)load" do
        @img.clear
        @img.append do
          if @cb.checked?
            image @el.text, cache: false
          else
            image @el.text
          end
        end
      end
      button "Show cache" do
        @cview.clear
        @cview.append do
          eb = edit_box width: 400
          DATABASE.open if DATABASE.closed?
          DATABASE.each do |key, value|
            eb.append "#{key} -> #{value}"
          end
        end
      end
      button "clear caches" do
        DATABASE.open if DATABASE.closed?
        DATABASE.each do |k, val|
          v = val.split('|')
          path = Shoes::image_cache_path v[1], File.extname(k)
          puts "delete: #{path}"
          File.delete path if File.exist? path
        end        
        DATABASE.clear
        DATABASE.close
      end
    end
    @img = flow {} 
    @cview = flow {}
  end
end
