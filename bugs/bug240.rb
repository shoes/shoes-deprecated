# issue #240 - canvas based widget click, release methods- rtn btn,l,t
Shoes.app do
  stack do
    flow do
      button "canvas" do
        @demo.clear do 
          stack width: 200, height: 200, 
              click: proc {|btn, t, l| puts "canvas clicked #{btn}, #{t}, #{l}"}, 
              release: proc {|btn, t, l| puts "canvas released #{btn}, #{t}, #{l}"} do
            background aliceblue
          end
        end
      end
      button "svg" do 
        @demo.clear do
          svg "brownshoes.svg", width: 200, height: 200,
            click: proc {|btn, t, l| puts "svg clicked #{btn}, #{t}, #{l}"},
            release: proc {|btn, t, l| puts "svg released #{btn}, #{t}, #{l}"}
        end
      end
      button "textblock" do
        @demo.clear do
          stack width: 200, height: 200 do
            para "textblock click"
            para "     click here", family: "mono", 
              click: proc {|btn, t, l| puts "textblock clicked #{btn}, #{t}, #{l}"},
              release: proc {|btn, t, l| puts "telextblock released #{btn}, #{t}, #{l}"}
          end
        end
      end
      button "image" do
        @demo.clear do
          image "../static/man-ele-textblock.png", 
            click: proc {|btn, t, l| puts "image clicked #{btn}, #{t}, #{l}"},
            release: proc {|btn, t, l| puts "image released #{btn}, #{t}, #{l}"}
        end
      end
      button "shape" do
        @demo.clear do
          stroke blue
          strokewidth 2
          fill burlywood
          oval 10, 10, 50, 
            click: proc {|btn, t, l| puts "shape clicked #{btn}, #{t}, #{l}"},
            release: proc {|btn, t, l| puts "shape released #{btn}, #{t}, #{l}"}
        end
      end
    end
  end
  @demo = stack width: 200, height: 200 do
  end
end
