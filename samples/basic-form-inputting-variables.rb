####FORMS WITH SHOES 1####

#This code demonstrates how to use a form to update a record.
#
#The user can see the current information stored, add new information, and then it is saved.
#
#It demonstrates how to use classes and variables with Shoes.
#
#Please borrow/steal/reuse/frankencode with this if it is useful
# - theUnmutual

Shoes.app do

class Queen
	attr_accessor	:name
	attr_accessor	:wigs
	attr_accessor	:diva
	
	def initialize(name,wigs,diva)
		@name = name
		@wigs = wigs
		@diva = diva
	end

end
	
	stack do
		title "Welcome, Queen!"
		para "Your agent sent us your details - but we seem to have them wrong."
		para "Please update our information"
		subtitle "Our information"

	#Creates a new Drag Queen, with three pieces of data
		@q = Queen.new("Helvetica Bold", 14, "Britney Spears")
		
	#Outputs the existing information
		
		para "Name:" + @q.name
		para "Number of wigs owned: " + @q.wigs.to_s
		para "Favourite diva:" + @q.diva	

		subtitle "Please update this!"
		subtitle "Press 'enter' to record the information"
		

	#User inputs new information	
		para "What is your name?"
			edit_line.finish = proc {|slf| @q.name = slf.text} 

		para "How many wigs do you own?"
			edit_line.finish = proc {|slf| @q.wigs = slf.text} 

		para "Who is your favourite diva?"
			edit_line.finish = proc {|slf| @q.diva = slf.text} 
			
	#The new information is saved each time you press 'enter'. This final step displays it in the program, so you can see it has worked:
	
			button "Update!" do
						append {para "Condragulations, #{@q.name}! We will make room for your #{@q.wigs} wigs, and prepare for your #{@q.diva} impression!"}
					end
	end
end
