Shoes.app do
	@my_stack = stack do
		background yellow
		para 'number 1'
		para 'number 2'
		para 'number 3'
		para 'number 4'
		para 'number 5'
	end
#	before(@my_stack.contents[2] ) do
  @my_stack.before(@my_stack.contents[2] ) do
		para 'number 1.5'
	end
end
