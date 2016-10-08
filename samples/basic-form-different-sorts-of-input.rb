####FORMS WITH SHOES 2####

#This form demonstrates how to update an existing record.
#Just like Forms 1
#
#This one also demonstrates how to use all 7 different input types.

Shoes.app do

class Apology
	attr_accessor	:recipient
	attr_accessor	:misdeed
	attr_accessor	:adverb
	attr_accessor	:slider
	attr_accessor	:feelings
	attr_accessor	:remorse
	
	def initialize(recipient,misdeed,adverb,slider,feelings,remorse)
		@recipient = recipient
		@misdeed = misdeed
		@adverb = adverb
		@slider = slider
		@feelings = feelings
		@remorse = remorse
	end

end
	
	stack do
	
	#Preamble text:
		title "The Apologiser"
		para "For help when you are sorry, oh so sorry..."
		subtitle "Produces an apology such as..."
		
	#Creates a new Apology, with standard data
		@a = Apology.new("Mother","eating all the cherries that were in the icebox","exquisitely","100","mortified","get you new metaphorical cherries")
		
	#Demonstrates current contents of the record:
		para "Dear #{@a.recipient}, \n I am #{@a.adverb} sorry for #{@a.misdeed}. I regret it #{@a.slider}%, and feel #{@a.feelings}. I will make it up to you by #{@a.remorse}."
		
		subtitle "Tell us about your misdeeds:"
		
		
		flow do			#List Box
				
				para "Who did you wrong?"

				list_box :items => ["baby", "Vicar", "Boris Johnson","Flight Lieutenant"] do |list|
								 @a.recipient = list.text
								end

		end
		
		flow do			#editline
		
			para "What did you do?"
				edit_line.finish = proc {|slf| @a.misdeed = slf.text} 
			inscription "Press enter to record your wrongdoing"
			
		end	

		stack do			#radio
=begin
	I'm stuck on radios, sorry! This code DOESN'T work...
	
	para "Approximately how sorry are you? \n"
			
			@rlist = ['slightly', 'deeply','lamentably','intolerably','vaguely']
				
			
				@rlist.map! do |adverbs|
					flow { @r = radio.checked=false; para adverbs }						 
					[@r, adverbs]																
																										
				end

=end			 
		end
		flow do			#slider
		
			para "But seriously, how sorry are you?"
		
	    @sl = slider fraction: 0.0 do |sd|
          @a.slider = sd.fraction * 100
					@p.text = "#{sd.fraction*100}% sorry"
      end
         @p = para "", margin_left: 10
        
		#this slider outputs hilarious numbers like 49.45454545454545454%.
		#You could force the number to round at this point before saving + displaying it...
				 
		
		end
		
		stack do			#checkbox
									#I don't understand this very well, this code has been smooshed in from the manual. It works, but I can't explain how.

			para "How do you feel about what you did?"
			
		#an array of bad feelings:
				@list = ['mortified', 'without hope','gloomy','despondant','undone','guilty','curious yellow','like a fool', 'like eating my own tongue', 'ashamed','pretty good about it actually']

		#We iterate through the array of bad feelings to display it, one feeling at a time:
			
				@list.map! do |feels|
					flow { @c = check; para feels }						#for each feeling in the array, a flow is displayed with the checkbox followed by the text of the feel 
					[@c, feels]																#I'm not certain what this does; it creates a new array for each line of the checklist
																										#I think it replaces each original feeling with an array row containing the checkbox, and the feeling string.
				end
	 
		end
		
		stack do			#editbox
			inscription "Complete the following sentence:"
			para "I will make it up to you by..."

					edit_box do |e|
						@a.remorse = e.text
					end			
		end
		
		para ""
		para "Press update when you think it's convincing enough"

			button "Update!" do
					#the next two lines check what checkboxes were selected, and join them into a string, then assign them to the Apology.
					#This has to be in the button section. Why? I guess because you want to "submit" your data all in one go, instead of it changing every time you click or unclick a box.
					
						selected = @list.map { |c, name| name if c.checked? }.compact			#creates a new array based on the old one, only containing ones where the box is checked.
						@a.feelings = selected.join(', ')																	#creates a string of all the checked feelings.
																																							#one could presumably get the checkbox to output as an array instead?
																																							#I will never be using checkboxes...
=begin
	Attempt to make radios work????
						rselected = @rlist.map { |r, name| name if r.checked? }.compact			
						@a.adverb = rselected																	
=end
																																							
					#this shows the completed letter, with the new information:
 						append {para "Dear #{@a.recipient}, \n I am #{@a.adverb} sorry for #{@a.misdeed}. I regret it #{@a.slider}%, and feel #{@a.feelings}. I will make it up to you by #{@a.remorse}."}
					end

	#IMPORTANT NOTE: with the exception of the checkbox, the button merely displays the information to prove to you that it has changed.
	#You can keep changing the information WITHOUT clicking "Update!", and it will still change the record. You just can't immediately see it.   
	#When creating an actual form, you may wish to write responses to a temporary array, and click "update!" to save; or do it this way, but link the user immediately to a new page.
		
	end #stack	
end #Shoes.app
