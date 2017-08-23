 Shoes.app do
   stack do
     title "Searching Google", :size => 16
     @status = para "One moment..."

     # Search Google for 'shoes' and print the HTTP headers
     download "http://www.google.com/search?q=shoes" do |goog|
       @status.text = "Headers: " + goog.response.headers.inspect
     end
   end
 end
