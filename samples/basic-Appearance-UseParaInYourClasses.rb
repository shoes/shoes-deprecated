#Using Shoes display methods in your classes
#
#
# You have a functioning Ruby program, and now you want to make it a Proper Program with Clickboxes And Stuff.
# But it's not working
#
# You changed all your puts to paras, but you keep getting "method para not found for class" errors
#
# This is because "para" is a sort of method which affects the Shoes app object - "do para to the app" i.e. "show my stuff"
# You get an error because the method para doesn't exist for Arrays, or for classes you created
#
# (Its all explained in the Manual under Hello!/Rules, but here's a practical example of fixing the problem:


#Here we have a Ruby custom method, which displays only strings from an array:

=begin

class Array

	def displaynames

		self.each do |ea|
	
			if ea.class == String
				puts ea 
			else
				#its a number, do nothing
			end
		
		end

	end
	
end

=end

#this doesn't work in Shoes. Here's how to make it work:


class Array

	def displaynames(shoes)  											#the method now takes an argument, which will be the Shoes app object

		self.each do |ea|
	
			if ea.class == String
				
				shoes.para ea 													#"shoes.para" tells the shoes app object to do para, i.e. show your content. 
				
				#WRONG: para ea
				#WRONG: puts ea
				#WRONG: Shoes::App::para ea
			else
				#its a number, do nothing
			end
		
		end

	end
	
end


Shoes.app do


mixedarray = ["Red",1,"blue",2,"green",3]


mixedarray.displaynames(self)										#this gives your custom method "self", which is always the app object.
																								#this is different from Ruby, slightly. It's explained in the manual




end


#This is one little example

#For a bigger program, simply go through and do these three steps every time you want to call a shoes method not in the body of Shoes.app:

#1. Make your method take an argument(shoes)
#2. When you call the method, pass in the argument self
#3. When you use shoes lingo outside of shoes, always write "shoes.alert" or "shoes.para" rather than just "alert" or "para"
