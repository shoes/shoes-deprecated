class MyApp < Shoes
	url "/", :setupscreen
	url "/entry", :entryscreen
	url "/help", :helpscreen
  
  def setupscreen
    stack do 
      para "Welcome to My Demo app"
      flow do
        button "entry" do visit '/entry'end
        button "help" do visit '/help' end
      end
    end
  end
  
  def entryscreen
    stack do 
      para "Entry Screen - whats your secret?"
      @the_secret = edit_line text: "I'm not telling you"
      flow do
        button "home" do visit '/'end
        button "help" do visit '/help' end
      end
    end
  end
  
  def helpscreen
     stack do 
      para "This page describes MyApp, a very demo for structuring a Shoes application "
      flow do
        button "entry" do visit '/entry'end
        button "home" do visit '/' end
      end
    end
  end
end
Shoes.app :width => 400, :height => 300, :margin => 5
