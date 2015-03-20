# Works ok with symbols instead of strings in Gtk2 (linux, win7)
# if the manual says otherwise, fix the manual.
# OSX bug 
Shoes.app do
  stack do
    flow {
      @r5 = radio; para "default"
      @r6 = radio; para "still more default"
    }
    flow {
      @r1 = radio :group, checked: true
      para "one"
    }
    flow {@r2 = radio :group; para "two"}
    para "================"
    flow { # horizontal grouping
      @r3 = radio :newgrp; para "near"
      @r4 = radio :newgrp; para "far"
    }
    @r4.checked = true 
    start { 
      para "checked one = #{@r1.checked?}\n" 
      para "checked far = #{@r4.checked?}\n"
    }
    button "Now check" do
      para "One: #{@r1.checked?}\n"
      para "Two: #{@r2.checked?}\n"
      para "near: #{@r3.checked?}\n"
      para "far: #{@r4.checked?}\n"
    end
  end
end
