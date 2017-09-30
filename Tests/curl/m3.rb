 Shoes.app do
   stack do
     title "GET Google", :size => 16
     @status = para "One moment..."

     download "http://www.google.com/search?q=shoes", 
         :method => "GET" do |dump|
       @status.text = dump.response.body
     end
   end
 end
