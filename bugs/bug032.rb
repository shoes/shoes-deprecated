Shoes.app {
   e = []
   e << button("button")
   e << edit_box
   e << edit_line
   e << radio
   e << check
   e << (list_box :items => [1, 2, 3])

   (0..e.size - 1).each { |n|
      timer(n * 5) {
         info "#{n * 5} seconds: #{e[n].class}"
         e[n].focus
      }
   }
}
