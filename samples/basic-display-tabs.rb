#FANCY WAYS OF DISPLAYING THINGS

#Displays different pages based on what you click. Demonstrates toggle and hide.

Shoes.app do


@s1 = stack :hidden=> true do						#starts hidden
	para "S1"
end

@s2 = stack do
	para "S2"
end

@s3 = stack do
	para "S3"
end


flow do
	@b1 = button "page one"
		@b1.click {@s1.toggle}
	@b2 = button "page two"
		@b2.click {@s2.toggle}
	@b3 = button "page three"
		@b3.click {@s3.toggle}
end


end
