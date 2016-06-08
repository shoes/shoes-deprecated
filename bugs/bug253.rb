# test some osx wanderers.
Shoes.app do
  stack do
    para "Check text editable"
    flow do
      @el = edit_line "FooBar Can't Edit", state: 'readonly'
      button "enable edit" do
       @el.state = nil
      end
    end
    flow do
      @eb = edit_box "No, don't try\nto edit this!!", state: "readonly"
      button "enable edit" do
        @eb.state = nil
      end
    end
  end 
end
