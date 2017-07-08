Shoes.app {
   stack(:top => 20) {
      [black, blue, red, green].each { |n|
         stack(:height => 80) { background n, scroll: true }
         stack(:height => 80) {}
      }
   }
   para
}
