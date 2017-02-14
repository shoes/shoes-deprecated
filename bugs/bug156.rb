Shoes.app do
  button('button'){alert 'hi'}
  msg = para 'show a key pressed'
  keypress{|k| msg.text = k}
end

