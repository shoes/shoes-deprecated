# Not a bug in Shoes 3.2/OSX 10.9.3
Shoes.app do
  @info = para "no button pressed."
  click do |button|
    @info.replace "#{button} was pressed."
  end
end
