Shoes.app do
  para "Do we have a button tooltip?"
  @btn = button "Quit", tooltip: "kill Shoes"  do
   Shoes.quit
  end
  flow do
    @chk = check tooltip: "check button will do something"
    para "watch"
  end
  @eb = edit_box tooltip: "enter something"
  @el = edit_line tooltip: "something you type into"
  @lb = list_box tooltip: "pull it"
  start do 
    @btn.tooltip = "This will will exit shoes properly" 
    @chk.tooltip = "This will alter reality"
    @eb.tooltip = "Something different"
    @el.tooltip = "you type here"
    @lb.tooltip = "Pick a card, any card"
  end
end
