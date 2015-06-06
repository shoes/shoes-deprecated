# remote debugger 
def Shoes.rdb
  Shoes.app height: 120, title: "Remote Debug" do 
    para "In a new terminal or console, run 'byebug -R localhost:8989'\n"
    para "Then use the spash screen to load the program that has the 'byebug'"
    para "command where you want to start the debugging from. Or"
    para "otherwise cause a 'byebug' to be invoked."
    require 'byebug'
    bye_thr = Thread.new {
      Byebug.wait_connection = true
      Byebug.start_server('localhost', 8989)
    }
  end
end
