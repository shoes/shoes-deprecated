# -*- encoding: utf-8 -*-
# crude shy-building UI; inspired by Jesse's "Shy Makey Thing"

require 'shoes/shy'

class Shoes
  def self.start_shy_builder(launch_script)
    launch_script = File.expand_path(launch_script)
    top_dir = File.dirname(launch_script)
    launch_script = File.basename(launch_script)
    shy_name = "#{top_dir}.shy" 
    Shoes.app do
      background white
      stack do
        para "Almost ready to make #{shy_name}"
        fields = {}
        for label, name in [["Project Name", "name"],
                            ["Version", "version"],
                            ["Your Name", "creator"]]
          flow :width => 1.0 do
            para "#{label}: "
            fields[name] = edit_line ''
          end
        end
        button "Build .shy" do
          shy_desc = Shy.new
          for name in fields.keys
            shy_desc.send("#{name}=".intern, fields[name].text)
          end
          shy_desc.launch = launch_script
          Shy.c(shy_name, shy_desc, top_dir)
          clear
          background white
          stack do
            para "Built #{shy_name}"
            button "Ok" do
              close
            end
          end
        end
      end
    end
  end
end
