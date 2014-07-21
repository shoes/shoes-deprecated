module TicTacToe
  class Cell < Shoes::Widget

    attr_reader :box, :letter

    def initialize(left, top)
      @box = rect(left, top, 40, 40, fill: white)
      #para
      @letter = para " ", top: top + 10, left: left + 15, size: 20, stroke: black, weight: 'bold'


      box.click do
        letter.replace app.player.to_s
      end
    end
  end
end
