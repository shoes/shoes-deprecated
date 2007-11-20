def dewikify_p(str)
  str = str.gsub(/\n+\s*/, " ").dump.
    gsub(/`(.+?)`/m, '", code("\1"), "').gsub(/\[\[BR\]\]/i, "\n").
    gsub(/'''(.+?)'''/m, '", strong("\1"), "').gsub(/''(.+?)''/m, '", em("\1"), "').
    gsub(/\[\[(\S+?) (.+?)\]\]/m, '", link("\2", :click => "\1"), "')
    # gsub(/\(\!\)/m, '<img src="/static/exclamation.png" />').
    # gsub(/\!\\(\S+\.png)\!/, '<img class="inline" src="/static/\1" />').
    # gsub(/\!(\S+\.png)\!/, '<img src="/static/\1" />')
  eval "[#{str}]"
end

def dewikify(str)
  proc do
    str.split(/\s*?(\{{3}(?:.+?)\}{3})|\n\n/m).map do |ps|
      next if ps.empty?
      if ps =~ /\{{3}(?:\s*\#![^\n]+)?(.+?)\}{3}/m
        para code($1), :stroke => "#636", :fill => "#eee", :size => 9 
      else
        case ps
        when /\A \* (.+)/m
          $1.split(/^ \* /).map { |x| para *dewikify_p(x) }
        when /\A==== (.+) ====/
          caption *dewikify_p($1)
        when /\A=== (.+) ===/
          tagline *dewikify_p($1)
        when /\A== (.+) ==/
          subtitle *dewikify_p($1)
        when /\A= (.+) =/
          title *dewikify_p($1)
        else
          para *dewikify_p(ps)
        end
      end
    end
  end
end

def Shoes.make_help_page(str)
  docs =
    (str.split(/^= (.+?) =/)[1..-1]/2).map do |k,v|
      sparts = v.split(/^== (.+?) ==/)
      sections = (sparts[1..-1]/2).map do |k2,v2|
        meth = v2.split(/^=== (.+?) ===/)
        [k2[/^(?:The )?([\-\w]+)/, 1],
         {'title' => k2,
          'description' => meth[0],
          'methods' => (meth[1..-1]/2).map { |_k,_v| [_k, _v] }}]
      end
      [k, {'description' => sparts[0], 'sections' => sections, 
         'class' => "toc" + k.downcase.gsub(/\W+/, '')}]
    end
  proc do
    style(Shoes::LinkHover, :stroke => red, :fill => nil)
    style(Shoes::Para, :size => 9)

    @doc = 
      stack :margin => 10, :margin_left => 130, :margin_top => 80,
        &dewikify(docs[0][-1]['description'])
    stack :top => 0, :left => 0 do
      stack do
        background black
        @title = title docs[0][0], :stroke => white, :margin => 10
      end
      @toc = {}
      stack :margin => 10, :width => 120 do
        docs.each do |sect_s, sect_h|
          sect_cls = sect_h['class']
          para strong(link(sect_s, :stroke => black) { 
              @toc.each { |k,v| v.send(k == sect_cls ? :show : :hide) }
              @title.replace sect_s
              @doc.clear(&dewikify(sect_h['description'])) 
            }), :size => 11, :align => "right"
          @toc[sect_cls] =
            stack :hidden => @toc.empty? ? false : true do
              links = sect_h['sections'].map do |meth_s, meth_h|
                [link(meth_s) {
                  @title.replace meth_h['title']
                  @doc.clear(&dewikify(meth_h['description'])) 
                  meth_h['methods'].each do |mname, expl|
                    @doc.append { tagline mname }
                    @doc.append(&dewikify(expl))
                  end
                }, "\n"]
              end.flatten
              links[-1] = {:size => 9, :align => "right"}
              para *links
            end
        end
      end
    end
  end
rescue => e
  p e.message
  p e.class
end

Shoes::Help = Shoes.make_help_page <<-'END'
= Shoes =

Shoes is a tiny graphics toolkit.  For making windowing programs in Ruby.  And, ideally, Shoes programs will run on any of the major platforms out there.  Microsoft Windows, Apple's Mac OS X, Linux and many others.

So, welcome to Shoes' built-in manual.  This manual is a Shoes program itself, written in Ruby.  This manual is unfinished, but I hope that it will be a complete reference in the near future.

==== What Can You Make With Shoes? ====

Well, you can make windowing applications.  But Shoes is inspired by the web, so applications tend to use images and text layout rather than a lot of widgets.  For example, Shoes doesn't come with tabbed controls or toolbars.  Shoes is a '''tiny''' toolkit, remember?

Still, Shoes does have a few widgets like buttons and edit boxes.  And many missing elements (like tabbed controls or toolbars) can be simulated with images and mouse events.

Shoes also has a very good art engine, for drawing with shapes and colors.  In this way, Shoes is inspired by NodeBox and Processing, two very good languages for drawing animated graphics.

== Built-in Methods ==

These methods can be used anywhere throughout Shoes programs.

All of these commands are unusual because you don't attach them with a dot.  
'''Every other method in this manual must be attached to an object with a dot.'''
But these are built-in methods (also called: Kernel methods.)  Which means no dot!

A common one is `puts`:

{{{
 #!ruby
 puts "No dots in sight"
}}}

Compare that to the method `reverse`, which isn't a Kernel method and is available with Arrays and Strings:

{{{
 #!ruby
 "Plaster of Paris".reverse
   #=> "siraP fo retsalP"
 [:dogs, :cows, :snakes].reverse
   #=> [:snakes, :cows, :dogs]
}}}

=== alert( message ) ===

Pops up a window containing a short message.

{{{
 #!ruby
 alert("I'm afraid I must interject!")
}}}

=== ask( message ) ===

Pops up a window and asks a question.  For example, you may want to ask someone their name.

{{{
 #!ruby
 name = ask("Please, enter your name:")
}}}

When the above script is run, the person at the computer will see a window with a blank box for entering their name.  The name will then be saved in the `name` variable.

=== ask_color(title: a String) ===

Pops up a color picker window.  The program will wait for a color to be picked, then gives you 
back a Color object.  See the `Color` help for some ways you can use this color.

{{{
 #!ruby
 backcolor = ask_color("Pick a background")
 Shoes.app do
   background backcolor
 end
}}}

=== ask_open_file() ===

Pops up an "Open file..." window.  It's the standard window which show all of your folders and lets you select a file to open.  Hands you back the name of the file.

{{{
 #!ruby
 filename = ask_open_file
 puts File.read(filename)
}}}

=== ask_save_file() ===

Pops up a "Save file..." window, similiar to `ask_open_file`, described above.

{{{
 #!ruby
 save_as = ask_save_file
}}}

=== confirm(question: a String) ===

Pops up a yes-or-no question.  If the person at the computer, clicks '''yes''', you'll get back a `true`.  If not, you'll get back `false`.

{{{
 #!ruby
 if confirm("Draw a circle?")
   oval :top => 0, :left => 0, :radius => 50
 end
}}}

=== exit() ===

Stops your program.  Call this anytime you want to suddenly call it quits.

== The App Object ==

An App is a single window running code at a URL.  When you switch URLs, a new App object is created and filled up with stacks, flows and other Shoes elements.

=== location() ===

Gets a string containing the URL of the current app.

== Slots: Stacks and Flows ==

Slots are boxes used to lay out images, text and so on.  The two most common slots are `stacks` and `flows`.  Slots can also be referred to as "boxes" or "canvases" in Shoes terminology.

END
