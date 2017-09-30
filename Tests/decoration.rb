Shoes.app do
  stack do
    para "Test decorations - title bar, resize controls"
    button "remove" do
      app.decorated = false
      @p.text = app.decorated? ? "True" : "False"
    end
    button "restore" do
      app.decorated = true
      @p.text = app.decorated? ? "True" : "False"
    end
    @p = para "True"
  end
end
