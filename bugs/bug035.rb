Shoes.app {
   click { |n, l, t| info "#{n}, (#{l}, #{t})" }
   (0..100).map { [*('A'..'Z')].sample(80) }.each_with_index { |n, i|
      para (i == 28 or i == 29) ? strong(n) : n
   }
}
