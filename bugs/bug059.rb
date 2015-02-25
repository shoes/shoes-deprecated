Shoes.app(:title => "Shoes Custom Ask & Alert", :width => 450, :height => 250) {
    #test default title - the hardcoded strings or new sytle setting.
    name = ask("Please enter your name:")
    alert "your name is #{name}"
    # test user supplied title
    name = ask("Please, enter your name:", title: "May i ?")
    alert "your name is #{name}", title: "Excellent !"
    # test no title at all
    name = ask("Please, enter your name:", title: nil)
    alert "your name is #{name}", title: nil
}
