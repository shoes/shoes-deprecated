@app = Shoes.app {
   s = stack { para "stack paragraph" }
   para "paragraph"
   info "contents: #{contents}"
   info "Shoes.APPS[0].contents: #{Shoes.APPS[0].slots.contents}"
   info "stack contents #{s.contents}"
   Shoes.app {
      info "Shoes.app(2)>>Shoes.APPS: #{Shoes.APPS}"
      begin
         info "Shoes.app(2)>>Shoes.APPS[0].contents: #{Shoes.APPS[0].slots.contents}"
      rescue Exception => e
         msg = e.backtrace.join("\n")
         error "Shoes.app(2)>>Shoes.APPS[0]:\n#{e.message}\n#{msg}"
      end
   }
}

Shoes.app {
   flow { para "flow paragraph" }
   info "Shoes.app(3)>>Shoes.APPS[1].contents: #{Shoes.APPS[1].slots.contents}"
   start do
     begin
      info "Shoes.app(3)>>Shoes.APPS: #{Shoes.APPS.inspect}"
     rescue Exception => e
      msg = e.backtrace.join("\n")
      error "Shoes.app(2)>>Shoes.APPS:\n#{e.message}\n#{msg}"
     end
     begin
      info "Shoes.app(3)>>Shoes.APPS[0].contents: #{Shoes.APPS[0].slots.contents}"
     rescue Exception => e
      msg = e.backtrace.join("\n")
      error "Shoes.app(2)>>Shoes.APPS[0]:\n#{e.message}\n#{msg}"
     end
   end
}

Shoes.show_log
