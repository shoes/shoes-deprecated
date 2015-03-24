Shoes.app(:width => 450, :height => 250) {
    #app.set_window_title("I'm not Shoes")
    name = ask("Please enter your name:")
    alert "your name is #{name}"
    ans = confirm "is the default title OK?"
    # test user supplied title
    #name = ask("Please, enter your name:", title: "May i ?")
    #name = ask("Please, enter your name:", title: 12345)
    #alert "Crash?", title: {:a => "hey"}
    #alert "your name is #{name}", title: "Excellent !"
    # test no title at all
    #name = ask("Please, enter your name:", title: nil)
    #alert "your name is #{name}", title: nil
    #ans = confirm "Do you think this works", :title => "I ask you"
    #alert "#{ans}", :title => "hashrocket says"
}
