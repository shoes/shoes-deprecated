Shoes.app do
  require 'shoes/data'
  require 'shoes/image'
  stack do
    flow do
      #@el = edit_line "#{DIR}/static/shoes-icon-walkabout.png"
      @el = edit_line "https://shoes.mvmanila.com/public/images/dino.jpg"
      @cb = check; para "Cached?"
      button "(Re)load" do
        @img.clear
        @img.append do
          image @el.text, cache: @cb.checked
        end
      end
      button "Show external cache" do
        @cview.clear
        @cview.append do
          eb = edit_box width: 400
          DATABASE.each do |key, value|
            eb.append "#{key} -> #{value}"
          end
        end
      end
      button "clear all caches" do
        app.cache_clear :all
        quit if confirm "Please restart Shoes for best results"
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
