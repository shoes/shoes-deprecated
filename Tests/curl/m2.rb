 Shoes.app do
   stack do
     title "Downloading Google image", :size => 16
     @status = para "One moment..."

     download "http://www.google.com/logos/nasa50th.gif",
       :save => "nasa50th.gif" do
         @status.text = "Okay, is downloaded."
     end
   end
 end

