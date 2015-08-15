Shoes.app title: "to_end test", height: 150 do
  @history, @pointer = [], 0

  keypress do |k|
      case k
      when :up
          @entry.text = @history[@pointer]
          #@entry.to_end
          @pointer -= 1 unless @pointer == 0
      when :down
          @pointer += 1 unless @pointer == @history.size-1
          @entry.text = @history[@pointer]
          @entry.to_end
      end
  end
  para "Enter words in Edit_Line below to fill the history stack\nTry KeyUp/KeyDown and Watch cursor ..."

  @entry = edit_line "", width: 0.9, margin: [10,10,0,10]
  @entry.finish = proc { |e| @history << e.text
                             @pointer = @history.size-1
                             e.text = ""
                        }

  timer(0) { @entry.focus }
end
