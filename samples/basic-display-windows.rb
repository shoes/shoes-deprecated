#WINDOWS TEST
#
#Shows how to launch a window which affects the main Shoes app. 

#Also shows how to launch and close windows

#Note! the window MUST be described in the same stack as you want to change!

Shoes.app do

	var = "
	"
	stack do
	
		p = para "hello!"
	
		button "Open a window" do
		
			window do               #this window has to be in the same stack as the para we are going to change. 
			
			
			
				edit_line do |line|
				var = line.text
				end
				
				button "change" do
				p.replace "#{var}"
				close
				end
	
			end
			
			
		end
	
	
	
	end


end
