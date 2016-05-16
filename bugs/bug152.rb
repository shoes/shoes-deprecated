# test for issue #152 
Shoes.app do
  stack do
   @s1 = stack do
     button "taze the perp!" do
       @s2.append { para "Zap!"}
     end
     button "remove evidence" do
       @f2.refresh_slot
     end
   end
   @s2 = stack do
     @f2 = flow do
       para "Don't taze me"
       para "Bro!"
     end
   end
  end
end
