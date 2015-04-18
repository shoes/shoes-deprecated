Shoes.app( :width => 450, :height => 250) {
    #test default title - the hardcoded strings "app-title says", "app-title asks"
    name = ask("Please enter your name")
    alert "your name is #{name}"
    # test user supplied title
    name2 = ask("Please, enter your name", title: "May i ?")
    alert "your name is #{name2}", title: "Excellent !"
    # test no title at all
    name3 = ask("Please, enter your name", title: nil)
    alert "your name is #{name3}", title: nil
}
