Shoes.app do
   flow do
      switch; para
   end
   
   flow do
      @n = switch(active: true) do
         @p.text = (@n.active? ? "true": "false") unless @p.nil?
      end
      @p = para
   end
   
   flow do
      @m = switch width: 80
      @m.click do
        #$stderr.puts "Click"
        @m.active? ? @e.start : @e.stop
      end
      @e = every(1) { |count| @q.text = count unless @q.nil? & @m.active? }
      @q = para ""
   end
   
   start do
      @e.stop
      @p.text = @n.active? ? "true" : "false"
      @m.active = false
   end
end
