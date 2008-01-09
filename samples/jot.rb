NOTES = [
  ['Welcome to the vJot Clone', 'Okay, you know, just type stuff in and it will save as you go.']
]

Shoes.app :title => "vJot Clone", 
  :width => 420, :height => 560, :resizable => false do

  @note = NOTES.first
  background "#C7EAFB"
  stack :width => 400, :margin => 20 do
    background "#eee", :radius => 12
    border "#00D0FF", :strokewidth => 3, :radius => 12
    stack :margin => 20 do
      caption "vJot Clone"
      @title = edit_line @note[0], :width => 1.0 do
        @note[0] = @title.text
        load_list
      end
      stack :width => 1.0, :height => 200 do
        background "#eee"
        @list = para
      end
      @jot = edit_box @note[1], :width => 1.0, :height => 200, :margin_bottom => 20 do
        @note[1] = @jot.text
      end
    end
  end

  def load_list
    @list.replace *(NOTES.map { |note|
      [link(note.first) { @note = load_note(note); load_list }, "\n"] 
    }.flatten +
      [link("+ Add a new Note") { NOTES << (@note = load_note); load_list }])
  end

  def load_note(note = ['New Note', ''])
    @note = note
    @title.text = note[0]
    @jot.text = note[1]
    note
  end

  load_list
end
