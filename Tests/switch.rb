Shoes.app do
   flow do
      switch; para
   end
   
   flow do
      @n = switch(active: true) do
         @p.text = @n.active? unless @p.nil?
      end
      @p = para
   end
   
   flow do
      @m = switch width: 80
      @m.click do
        @m.active? ? @e.start : @e.stop
      end
      @e = every(1) { |count| @q.text = count unless @q.nil? }
      @q = para
   end
   
   start do
      @e.stop
      @p.text = @n.active?
      @m.active = true
   end
end
