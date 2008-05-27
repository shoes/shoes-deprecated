module Accordion
  def open_page stack
    active = contents.map { |x| x.contents[1] }.detect { |x| x.height > 0 }
    return if active == stack
    append do
      a = animate 30 do
        stack.height += 20
        active.height -= 20 if active
        a.stop if stack.height > 120
      end
    end
  end
  def page title, text
    @pages ||= []
    @pages <<
      stack do
        page_text = nil
        stack :width => "100%" do
          background "#eee"
          para link(title).click { open_page page_text }
        end
        page_text =
          stack :width => "100%", :height => 0 do
            text.split(/\n+/).each do |pg|
              para pg
            end
          end
      end
  end
end

Shoes.app do
  extend Accordion
  page "Common descent", <<-'END'
A group of organisms is said to have common descent if they have a common ancestor. In biology, the theory of universal common descent proposes that all organisms on Earth are descended from a common ancestor or ancestral gene pool.

A theory of universal common descent based on evolutionary principles was proposed by Charles Darwin in his book The Origin of Species (1859), and later in The Descent of Man (1871). This theory is now generally accepted by biologists, and the last universal common ancestor (LUCA or LUA), that is, the most recent common ancestor of all currently living organisms, is believed to have appeared about 3.9 billion years ago. The theory of a common ancestor between all organisms is one of the principles of evolution, although for single cell organisms and viruses, single phylogeny is disputed.
END
  page "History", <<-'END'
The first suggestion that all organisms may have had a common ancestor and diverged through random variation and natural selection was made in 1745 by the French mathematician and scientist Pierre-Louis Moreau de Maupertuis (1698-1759) in his work Venus physique. Specifically:

"Could one not say that, in the fortuitous combinations of the productions of nature, as there must be some characterized by a certain relation of fitness which are able to subsist, it is not to be wondered at that this fitness is present in all the species that are currently in existence? Chance, one would say, produced an innumerable multitude of individuals; a small number found themselves constructed in such a manner that the parts of the animal were able to satisfy its needs; in another infinitely greater number, there was neither fitness nor order: all of these latter have perished. Animals lacking a mouth could not live; others lacking reproductive organs could not perpetuate themselves... The species we see today are but the smallest part of what blind destiny has produced..."

In 1790, Immanuel Kant (Konigsberg (Kaliningrad) 1724 - 1804), in his Kritik der Urtheilskraft, states that the analogy of animal forms implies a common original type and thus a common parent.
END
end
