# Ruby symbols last forever so the group lasts forever
Shoes.app do
  @s = stack do
   @r1 = radio :items
   @r2 = radio :items
  end

  button "Clear" do
    @s.clear do
      @r1 = radio :items
      @r2 = radio :items
   end
 end
end

