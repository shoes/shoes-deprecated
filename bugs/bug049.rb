Shoes.app {
   para Shoes::FONTS
   para "\n----------------------------------------------\n"
   para "#{Shoes::FONTS}\n"
   para "\n----------------------------------------------\n"
   font "#{DIR}/fonts/Lacuna.ttf"
   para "#{Shoes::FONTS}\n"
   para "\n..............................................\n"
   Shoes::FONTS.each {|ft| info ft}
}
