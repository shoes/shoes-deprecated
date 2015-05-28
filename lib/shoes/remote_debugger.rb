# remote debugger 
def Shoes.rdb
  Shoes.app height: 80 do 
    para "In a new terminal or console, run 'byebug -R localhost:8989'\n"
    para "The use the spash screen to load the program that has the 'byebug'\n"
    para "command where you want to start the debuggin"
    require 'byebug'
    bye_thr = Thread.new {
      Byebug.wait_connection = true
      Byebug.start_server('localhost', 8989)
    }
  end
end
