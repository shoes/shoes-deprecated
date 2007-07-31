Shoes.app :width => 300, :height => 150, :margin => 10 do
  def answer(v)
    @answer.replace "<span color='black'>#{v.inspect}</span>"
  end

  button "Ask" do
    answer ask("What is your name?")
  end
  button "Confirm" do
    answer confirm("Would you like to proceed?")
  end
  button "Open File..." do
    answer ask_open_file
  end
  button "Save File..." do
    answer ask_save_file
  end
  button "Color" do
    answer ask_color("Pick a Color")
  end

  @answer = text "Answers appear here"
end
