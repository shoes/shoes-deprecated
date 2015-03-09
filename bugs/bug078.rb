Shoes.app do
  stack do
    para "Shoes default - Arial according to manual"
    para "Serif font so I'm told", :family => "serif"
    para "Sans-Serif if we know what to say", :family => 'sans-serif'
    para "This one is monospace", :family => "monospace"
    para "Deprecated name is 'sans serif'", :family => 'sans'
    para "and this one uses mono deprecated name", :family =>"mono"
    button "What font is this?"
    para "Lets use a MS font and see what happens."
    para "Perhaps Palatino works", :font =>"Palatino Linotype"
  end
end
