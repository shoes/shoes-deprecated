require_relative 'lib/tic_tac_toe.rb'

Shoes.app width: 220, height: 250 do
  extend TicTacToe
  new_game
end
