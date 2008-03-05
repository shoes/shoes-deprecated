module Shoes::LogWindow
  def setup
    stack do
      flow do
        background black
        stack :width => -100 do
          tagline strong("SHOES LOG WINDOW"), :stroke => white
        end
        button "Clear", :margin => 6, :width => 80, :height => 40 do
          Shoes.log.clear
        end
      end
      @log, @hash = stack, nil
      update
      every(1) do
        update
      end
    end
  end
  def update
    if @hash != Shoes.log.hash
      @hash = Shoes.log.hash
      @log.clear do
        i = 0
        Shoes.log.each do |typ, msg, at|
          stack do
            background "#eee" if i % 2 == 0
            inscription at, :stroke => "#905", :margin => 4, :margin_bottom => 0
            para strong(typ), " #{msg}", :margin => 4, :margin_top => 0
          end
          i += 1
        end
      end
    end
  end
end
