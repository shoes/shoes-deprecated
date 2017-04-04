Shoes.app do
   flow do
      @n = spinner
      @m = spinner
      @o = spinner start: true
   end
   
   click do |btn, left, top|
      @m.started? ? @m.stop : @m.start
   end
   
   start do
      @n.start
   end
end
