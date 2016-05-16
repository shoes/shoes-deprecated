Shoes.app do
  stack do
    all_fonts = Shoes::FONTS
    para "#{all_fonts.length} fonts [0]: #{all_fonts[0]} [-1]: #{all_fonts[-1]}"
    para "Lacuna? #{all_fonts.include? 'Lacuna Regular'}"
    lac = font "#{DIR}/fonts/Lacuna.ttf"
    para "Loaded: #{lac.inspect}"
    new_fonts = Shoes::FONTS
    para "#{new_fonts.length} fonts [0]: #{new_fonts[0]} [-1]: #{new_fonts[-1]}"
    para "Lacuna? #{new_fonts.include? 'Lacuna Regular'}" 
  end
end
