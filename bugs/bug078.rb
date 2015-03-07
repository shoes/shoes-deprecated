Shoes.app do
  para "Shoes default - Arial according to manual\n"
  para "Serif font so I'm told\n", :font => "serif"
  para "Sans-Serif if we know what to say\n", :font => 'sans-serif'
  para "This one is monospace\n", :font => "monospace"
  para "Deprecated name is 'sans serif'\n", :fonts => 'sans serif'
  para "Deprecated name is 'sans serif'\n", :fonts => 'sans-serif'
  para "Pretty much the same - 'sans'\n", :fonts => 'Sans'
  para "and this one uses mono deprecated name\n", :font=>"mono"
  button "If Serif what now"
end
