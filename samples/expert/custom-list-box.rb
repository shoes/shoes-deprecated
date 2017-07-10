require("observer")

class Items
  include Observable
  
  def initialize(array)
    @array = array
  end
  
  def method_missing(meth, *args, &block)
    @array.send(meth, *args, &block)
    changed
    notify_observers(@array, meth, *args, &block)
  end
end

class CustomListBox < Shoes::Widget
  attr_accessor :items
  
  def initialize(options = {}, &block)
    @lb = list_box options, &block
    @items = Items.new(@lb.items)
    @items.add_observer(self)
  end
  
  def method_missing(meth, *args, &block)
    if @lb.respond_to? meth
      @lb.send(meth, *args, &block)
    else
      super
    end
  end
  
  def update(array, *call)
    @lb.items = array
  end
end

Shoes.app {
  @p = para "Any selection?"
  @l = custom_list_box :items => ["first", "second"], :choose => "first" do |n|
    @p.replace "Selection is #{n.text}."
  end
  @e = edit_line

  button "ok" do
    @l.items << @e.text
  end
  
  button "collect" do
    @l.items.collect! { |n| n.reverse }
  end
}