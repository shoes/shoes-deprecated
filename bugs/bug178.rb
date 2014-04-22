# Scroll wheel behaviour - shoes/gtk2 
# scroll wheel only works when the cursor is in the scrollbar
# other apps allow the wheel to scroll when the cursor is over the widget 
# managed by the scrollbar. I think shoes/gtk should be similar.
Shoes.app do
  stack :height => 100, :width => 100, :scroll => true do
    para "A tremor in the Force. The last time I felt it was in the presence of my old master. I can't get involved! I've got work to do! It's not that I like the Empire, I hate it, but there's nothing I can do about it right now. It's such a long way from here. What!? I suggest you try it again, Luke. This time, let go your conscious self and act on instinct. I call it luck. I want to come with you to Alderaan. There's nothing for me here now. I want to learn the ways of the Force and be a Jedi, like my father before me."
  end
end
