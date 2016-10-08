#LINK TEST
#
#This app demonstrates how to create interlinked pages on Shoes.
#It's copied straight from the Nobody Knows Shoes book. 

#remember that each of these pages is its own method. 
#this means data created/stored in one is NOT available on other pages

#encapsulation, yo.

#to use the same data across different pages, you need to SAVE your data on each page, and RELOAD it on each new page.


class Booklist < Shoes
	url '/',					:index
	url '/twain',			:twain
	url '/orwell',		:orwell										#i just learned that all methods are symbols. Are we creating emthods here?
	url '/tolstoy',		:tolstoy
	
	def index
		para "Books Ive read: ",
			link("by Mark Twain",	:click => "/twain"),
			link("by George Orwell", :click => "/orwell"),
			link("by Leo Tolstoy", :click => "/tolstoy")
	end
	
	def twain
		title "The Title"
		para "Just huck finn. \n",
			link("Go back.", :click => "/")
	end
	
	def orwell
		para "Catalonia, 1984 \n",													#this comma is v important, or the link wont show
			link("Go back.", :click => "/")										#I think this is because links are text, and therefore somehow "included" in the paragraph
																												#the comma tells the program to keep looking for the next thing to display
																												
	end
	
	def tolstoy
		stack do
			para "Anna karenina",														#example I added to show it working nicely in a stack
			link("Go back.", :click => "/")									
		end
	end
	
	
end

Shoes.app :width =>400, :height => 500



