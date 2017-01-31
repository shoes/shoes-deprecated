# bug 282 accept title for ask_open/save_file/folder
# we don't care about the answer just that the dialogs have
# the proper title
Shoes.app do
  stack do 
    para "Open dialogs"
    flow do 
      button "Open file w/o" do
        ask_open_file
      end
      button "Open file with" do
        ask_open_file title: "open Shoes test file"
      end
      button "Open folder w/o" do
        ask_open_folder 
      end
      button "Open folder with" do
        ask_open_folder title: "open shoes folder"
      end
    end
    para "Save dialogs"
    flow do
	  button "Save File w/o" do
	    ask_save_file
	  end
	  button "Save File with" do
	    ask_save_file title: "Funky Shoes save file"
	  end
	  button "Save Folder w/o" do
	    ask_save_folder 
	  end
	  button "Save folder with" do
	    ask_save_folder title: "crappy save folder here"
	  end
	end
  end
end
