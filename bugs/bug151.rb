Shoes.app do
  slot = flow do 
    para 'Click me!'
    click { slot.remove }
  end
end
