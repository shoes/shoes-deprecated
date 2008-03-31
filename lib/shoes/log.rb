module Shoes::LogWindow
  def setup
    stack do
      flow do
        background black
        stack :width => -100 do
          tagline "Shoes Console", :stroke => white
        end
        button "Clear", :margin => 6, :width => 80, :height => 40 do
          Shoes.log.clear
        end
      end
      @log, @hash = stack, nil
      update
      every(0.2) do
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
            background "#f1f5e1" if i % 2 == 0
            inscription strong(typ, :stroke => "#C50"), " ", at, :stroke => "#950", :margin => 4, :margin_bottom => 0
            flow do
              stack :margin => 4, :width => 20 do
                image "#{DIR}/static/icon-#{typ}.png"
              end
              stack :margin => 4, :width => -20 do
                para " #{msg}", :margin => 4, :margin_top => 0
              end
            end
          end
          i += 1
        end
      end
    end
  end
end
