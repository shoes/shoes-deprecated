#STYLESHEET TEST

#To make code easier to maintain, don't style every para and title individually.

#Just write out your style at the top and save it as a constant, and apply that style to everything you want to look the same.

#This way, you can change the whole program by just swapping one word or number about.

Shoes.app do
	SPECIAL_STYLE = {:size => 9, :margin => 12, :font =>'Georgia'}
	HEADER_STYLE = {:size=>40,:font=> 'Times New Roman'}
	STACK_STYLE = {:width=>10}
	
stack do

	para "This para is styled", SPECIAL_STYLE
	para "This para is not"



end

stack do

	title "this title is styled", SPECIAL_STYLE
	title "this title is not"
	para "---------------------------------------------------------------"
end


stack do
	title "This is styled with our special header style", HEADER_STYLE
	para "And here is a paragraph, styled the old fashioned way", :font=>'Georgia'
end

stack STACK_STYLE do
	para "here is the same trick done with a stack. Groovy!"
end
end
