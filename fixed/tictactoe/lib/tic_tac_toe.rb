require_relative 'cell'

module TicTacToe
  # if you want to use an image instead
  #  X = "../static/x.png"
  #  O = "../static/o.png"

  attr_reader :player

  def new_game
    title "Tic-Tac-Toe", left: 20, top: 10

    generate_cells
    @player = :x

    button "New Game", top: 200, left: 60  do
      app.clear do
        new_game
      end
    end
  end

  def generate_cells
    [50, 90, 130].each do |left|
      [60, 100, 140].each do |top|
        cell left, top
      end
    end
  end
end
