module Shoes::Manual
  CODE_STYLE = {:size => 9, :margin => 12}
  INTRO_STYLE = {:size => 12, :weight => "bold", :margin_bottom => 20, :stroke => "#000"}

  def dewikify_p(str)
    str = str.gsub(/\n+\s*/, " ").dump.
      gsub(/`(.+?)`/m, '", code("\1"), "').gsub(/\[\[BR\]\]/i, "\n").
      gsub(/'''(.+?)'''/m, '", strong("\1"), "').gsub(/''(.+?)''/m, '", em("\1"), "').
      gsub(/\[\[(\S+?) (.+?)\]\]/m, '", link("\2", :click => "\1"), "')
      # gsub(/\(\!\)/m, '<img src="/static/exclamation.png" />').
      # gsub(/\!\\(\S+\.png)\!/, '<img class="inline" src="/static/\1" />').
      # gsub(/\!(\S+\.png)\!/, '<img src="/static/\1" />')
    eval("[#{str}]")
  end

  def dewikify(str, intro = false)
    proc do
      paras = str.split(/\s*?(\{{3}(?:.+?)\}{3})|\n\n/m).reject { |x| x.empty? }
      if intro
        para *(dewikify_p(paras.shift) + [INTRO_STYLE])
      end
      paras.map do |ps|
        if ps =~ /\{{3}(?:\s*\#![^\n]+)?(.+?)\}{3}/m
          stack :margin_bottom => 12 do 
            background rgb(210, 210, 210)
            para code($1.gsub(/\A\n+/, '').chomp), CODE_STYLE 
          end
        else
          case ps
          when /\A \* (.+)/m
            para *(dewikify_p($1.split(/^ \* /).join("[[BR]]")))
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
    extend Shoes::Manual
    style(Code, :weight => "bold", :stroke => "#C30")
    style(LinkHover, :stroke => green, :fill => nil)
    style(Para, :size => 9, :stroke => "#332")
    style(Tagline, :size => 12, :weight => "bold", :stroke => "#eee", :margin => 6)
    background "#ddd".."#fff", :angle => 90

    stack do
      background black
      para "The Shoes Manual", :stroke => "#eee", :margin_top => 8, :margin_left => 17, 
        :margin_bottom => 0
      @title = title docs[0][0], :stroke => white, :margin => 4, :margin_left => 14,
        :margin_top => 0, :weight => "bold"
      background "rgb(66, 66, 66, 180)".."rgb(0, 0, 0, 0)", :height => 0.7
      background "rgb(66, 66, 66, 100)".."rgb(255, 255, 255, 0)", :height => 20, :bottom => 0
    end
    @doc =
      stack :margin_left => 130, :margin_top => 20, :margin_bottom => 50, :margin_right => 20 + gutter,
        &dewikify(docs[0][-1]['description'], true)
    stack :top => 80, :left => 0, :sticky => Window do
      @toc = {}
      stack :margin => 12, :width => 130, :margin_top => 20 do
        background "#eee", :radius => 4
        docs.each do |sect_s, sect_h|
          sect_cls = sect_h['class']
          para strong(link(sect_s, :stroke => black) { 
              @toc.each { |k,v| v.send(k == sect_cls ? :show : :hide) }
              @title.replace sect_s
              @doc.clear(&dewikify(sect_h['description'], true)) 
              self.scroll_top = 0
            }), :size => 11, :margin => 4
          @toc[sect_cls] =
            stack :hidden => @toc.empty? ? false : true do
              links = sect_h['sections'].map do |meth_s, meth_h|
                [link(meth_s) {
                  @title.replace meth_h['title']
                  @doc.clear(&dewikify(meth_h['description'], true)) 
                  @doc.append do
                    meth_h['methods'].each do |mname, expl|
                      stack(:margin_top => 8, :margin_bottom => 8) { 
                        background "#333".."#666", :radius => 3, :angle => 90; tagline mname, :margin => 4 }
                      instance_eval &dewikify(expl)
                    end
                  end
                  self.scroll_top = 0
                }, "\n"]
              end.flatten
              links[-1] = {:size => 9, :margin => 4}
              para *links
            end
        end
      end
      stack :margin => 12, :width => 118 do
        inscription "Shoes #{Shoes::RELEASE_NAME}\nRevision: #{Shoes::REVISION}",
          :size => 7, :align => "center", :stroke => "#999"
      end
    end
    image "#{DIR}/static/shoes-icon.png", :top => 8, :right => 10 + gutter,
      :width => 64, :height => 64
  end
rescue => e
  p e.message
  p e.class
end

Shoes::Help = Shoes.make_help_page <<-'END'
= Hello! =

Shoes is a tiny graphics toolkit. It's simple and straightforward. Shoes was born to be easy!  Really, it was made for absolute beginners. There's really nothing to it.

You see, the trivial Shoes program can be just one line:

{{{
 #!ruby
 Shoes.app { button("Click me!") { alert("I am so proud of you...") } }
}}}

And, ideally, Shoes programs will run on any of the major platforms out there. Microsoft Windows, Apple's Mac OS X, Linux and many others.

So, welcome to Shoes' built-in manual. This manual is a Shoes program itself, written in Ruby. This manual is unfinished, but I hope that it will be a complete reference in the near future.

==== What Can You Make With Shoes? ====

Well, you can make windowing applications. But Shoes is inspired by the web, so applications tend to use images and text layout rather than a lot of widgets. For example, Shoes doesn't come with tabbed controls or toolbars. Shoes is a ''tiny'' toolkit, remember?

Still, Shoes does have a few widgets like buttons and edit boxes. And many missing elements (like tabbed controls or toolbars) can be simulated with images.

Shoes also has a very good art engine called Cairo, which is used for drawing with shapes and colors. In this way, Shoes is inspired by NodeBox and Processing, two very good languages for drawing animated graphics.

== Built-in Methods ==

These methods can be used anywhere throughout Shoes programs.

All of these commands are unusual because you don't attach them with a dot. 
'''Every other method in this manual must be attached to an object with a dot.'''
But these are built-in methods (also called: Kernel methods.) Which means no dot!

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

==== Built-in Constants ====

Shoes also has a handful of built-in constants which may prove useful if you are trying to sniff out what release of Shoes is running.

'''Shoes::RELEASE_NAME''' contains a string with the name of the Shoes release.  All Shoes releases are named, starting with Curious.

'''Shoes::RELEASE_ID''' contains a number representing the Shoes release.  So, for example, Curious is number 1, as it was the first official release.

'''Shoes::REVISION''' is the Subversion revision number for this build.

=== alert( message ) » nil ===

Pops up a window containing a short message.

{{{
 #!ruby
 alert("I'm afraid I must interject!")
}}}

=== ask( message ) » String ===

Pops up a window and asks a question. For example, you may want to ask someone their name.

{{{
 #!ruby
 name = ask("Please, enter your name:")
}}}

When the above script is run, the person at the computer will see a window with a blank box for entering their name. The name will then be saved in the `name` variable.

=== ask_color(title: a String) » Shoes::Color ===

Pops up a color picker window. The program will wait for a color to be picked, then gives you 
back a Color object. See the `Color` help for some ways you can use this color.

{{{
 #!ruby
 backcolor = ask_color("Pick a background")
 Shoes.app do
  background backcolor
 end
}}}

=== ask_open_file() » String ===

Pops up an "Open file..." window. It's the standard window which show all of your folders and lets you select a file to open. Hands you back the name of the file.

{{{
 #!ruby
 filename = ask_open_file
 puts File.read(filename)
}}}

=== ask_save_file() » String ===

Pops up a "Save file..." window, similiar to `ask_open_file`, described above.

{{{
 #!ruby
 save_as = ask_save_file
}}}

=== confirm(question: a String) » true or false ===

Pops up a yes-or-no question. If the person at the computer, clicks '''yes''', you'll get back a `true`. If not, you'll get back `false`.

{{{
 #!ruby
 if confirm("Draw a circle?")
  oval :top => 0, :left => 0, :radius => 50
 end
}}}

=== exit() ===

Stops your program. Call this anytime you want to suddenly call it quits.

=== gradient(color1, color2) » Shoes::Pattern ===

Builds a linear gradient from two colors.  For each color, you may pass in a Shoes::Color object or a string describing the color.

=== gray(the numbers: darkness, alpha) » Shoes::Color ===

Create a grayscale color from a level of darkness and, optionally, an alpha level.

{{{
black = gray(0.0)
white = gray(1.0)
}}}

=== rgb(a series of numbers: red, green, blue, alpha) » Shoes::Color ===

Create a color from red, green and blue components.  An alpha level (indicating transparency) can also be added, optionally.

When passing in a whole number, use values from 0 to 255.

{{{
blueviolet = rgb(138, 43, 226)
darkgreen = rgb(0, 100, 0)
}}}

Or, use a decimal number from 0.0 to 1.0.

{{{
blueviolet = rgb(0.54, 0.17, 0.89)
darkgreen = rgb(0, 0.4, 0)
}}}

This method may also be called as `Shoes.rgb`.

== The App Object ==

An App is a single window running code at a URL. When you switch URLs, a new App object is created and filled up with stacks, flows and other Shoes elements.

The App itself, in slot/box terminology, is a flow.  See the ''Slots'' section for more, but this just means that any elements placed directly at the top-level will flow.

=== close() ===

Closes the app window.  If multiple windows are open and you want to close the entire application, use the built-in method `exit`.

=== location() » String ===

Gets a string containing the URL of the current app.

= Slots =

Slots are boxes used to lay out images, text and so on. The two most common slots are `stacks` and `flows`. Slots can also be referred to as "boxes" or "canvases" in Shoes terminology.

Since the mouse wheel and PageUp and PageDown are so pervasive on every platform, vertical scrolling has really become the only overflow that matters. So, in Shoes, just as on the web, width is generally fixed. While height goes on and on.

Now, you can also just use specific widths and heights for everything, if you want. That'll take some math, but everything could be perfect.

Generally, I'd suggest using stacks and flows. The idea here is that you want to fill up a certain width with things, then advance down the page, filling up further widths. You can think of these as being analogous to HTML's "block" and "inline" styles. 

==== Stacks ====

A stack is simply a vertical stack of elements. Each element in a stack is placed directly under the element preceding it.

A stack is also shaped like a box. So if a stack is given a width of 250, that stack is itself an element which is 250 pixels wide. 

==== Flows ====

A flow will pack elements in as tightly as it can. A width will be filled, then will wrap beneath those elements. Text elements placed next to each other will appear as a single paragraph. Images and widgets will run together as a series.

Like the stack, a flow is a box. So stacks and flows can safely be embedded and, without respect to their contents, are identical. They just treat their contents differently.

Last thing: The Shoes window itself is a flow. 

== Art for Slots ==

Each slot is like a canvas, a blank surface which can be covered with an assortment of colored shapes or gradients.

Many common shapes can be drawn with methods like `oval` and `rect`.  You'll need to set up the paintbrush colors first, though.

The `stroke` command sets the line color.  And the `fill` command sets the color used to paint inside the lines.

{{{
 #!ruby
 Shoes.app do
   stroke red
   fill blue
   oval :top => 10, :left => 10,
     :radius => 100
 end
}}}

That code gives you a blue pie with a red line around it.  One-hundred pixels wide, placed just a few pixels
southeast of the window's upper left corner.

The `blue` and `red` methods above are Color objects.  See the section on Colors for more on how to mix
colors.

==== Inspiration from Processing and NodeBox ====

The artful methods generally come verbatim from NodeBox, a drawing kit for Python.  In turn, NodeBox gets much of its ideas from Processing, a Java-like language for graphics and animation.  I owe a great debt to the creators of these wonderful programs!

Shoes does a few things differently from NodeBox and Processing.  For example, Shoes has different color methods, including having its own Color objects, though these are very similar to Processing's color methods.  And Shoes also allows images and gradients to be used for drawing lines and filling in shapes.

Shoes also borrows some animation ideas from Processing and will continue to closely consult Processing's methods as it expands.

=== arrow(x, y, width) » Shoes::Shape ===

Draws an arrow at coordinates (x, y) with a pixel `width`.

=== fill(pattern) » pattern ===

Sets the fill bucket to a specific color (or pattern.)  Patterns can be colors, gradients or images.  So, once the fill bucket is set, you can draw shapes and they will be colored in with the pattern you've chosen.

To draw a star with an image pattern:

{{{
 #!ruby
 Shoes.app do
   fill "images/shiny.png"
   star 200, 200, 5
 end
}}}

To clear the fill bucket, use `nofill`.  And to set the line color (the border of the star,) use the `stroke` method.

=== nofill() » self ===

Blanks the fill color, so that any shapes drawn will not be filled in.  Instead, shapes will have only a
lining, leaving the middle transparent.

=== nostroke() » self ===

Empties the line color.  Shapes drawn will have no outer line.  If `nofill` is also set, shapes drawn will
not be visible.

=== line(x, y, x2, y2) » Shoes::Shape ===

Draws a line using the current line color (aka "stroke") starting at coordinates (x, y) and ending at coordinates (x2, y2).

=== oval(top, left, radius) » Shoes::Shape ===

Draws a circular form at pixel coordinates (top, left) with a width and height of `radius` pixels.  The line and fill colors are used to draw the shape.

{{{
 #!ruby
 Shoes.app do
   stroke blue
   strokewidth 4
   fill black

   oval 10, 10, 50
 end
}}}

To draw an oval of varied proportions, you may also use the syntax: `oval(top, left, width, height)`.

=== oval(styles) » Shoes::Shape ===

Draw circular form using a style hash.  The following styles are supported:

 * `top`: the y-coordinate for the oval pen.
 * `left`: the x-coordinate for the oval pen.
 * `radius`: the width and height of the circle.
 * `width`: a specific pixel width for the oval.
 * `height`: a specific pixel height for the oval.
 * `center`: do the coordinates specific the oval's center? (true or false)

These styles may also be altered using the `style` method on the Shape object.

=== rect(top, left, width, height, corners = 0) » Shoes::Shape ===

Draws a rectangle starting from coordinates (top, left) with dimensions of width x height.  Optionally, you may give the rectangle rounded corners with a fifth argument: the radius of the corners in pixels.

As with all other shapes, the rectangle is drawn using the stroke and fill colors.

{{{
 #!ruby
 Shoes.app do
   stroke rgb(0.5, 0.5, 0.7)
   fill rgb(1.0, 1.0, 0.9)
   rect 10, 10, self.width - 10, self.height - 10
 end
}}}

The above sample draws a rectangle which fills the area of its parent box, leaving a margin of 10 pixels around the edge.  Also see the `background` method for a rectangle which defaults to filling its parent box.

=== rect(styles) » Shoes::Shape ===

Draw a rectangle using a style hash.  The following styles are supported:

 * `top`: the y-coordinate for the rectangle.
 * `left`: the x-coordinate for the rectangle.
 * `radius`: the pixel radius of the rectangle's corners.
 * `width`: a specific pixel width for the rectangle.
 * `height`: a specific pixel height for the rectangle.
 * `center`: do the coordinates specific the rectangle's center? (true or false)

These styles may also be altered using the `style` method on the Shape object.

=== shape(x, y) { ... } » Shoes::Shape ===

Describes an arbitrary shape to draw, beginning at coordinates (x, y) and continued by calls to `line_to`, `curve_to` and `move_to` inside the block.

=== star(x, y, points = 10, outer = 100.0, inner = 50.0) » Shoes::Shape ===

Draws a star using the stroke and fill colors.  The star is positioned at coordinates (x, y) with a certain number of `points`.  The `outer` width defines the full radius of the star; the `inner` width specifies the radius of the star's middle, where points stem from.

=== stroke(pattern) » pattern ===

Set the active line color for this slot.  The `pattern` may be a color, a gradient or an image, all of which are categorized as "patterns."  The line color is then used to draw the borders of any subsequent shape.

So, to draw an arrow with a red line around it:

{{{
 #!ruby
 Shoes.app do
   stroke red
   arrow 0, 100, 10
 end
}}}

To clear the line color, use the `nostroke` method.

=== strokewidth(a number) » self ===

Sets the line size for all drawing within this slot.  Whereas the `stroke` method alters the line color, the `strokewidth` method alters the line size in pixels.  Calling `strokewidth(4)` will cause lines to be drawn 4 pixels wide.

=== transform(:center or :corner) » self ===

Should transformations (such as `skew` and `rotate`) be performed around the center of the shape?  Or the corner of the shape?  Shoes defaults to `:corner`.

== Element Creation ==

Shoes has a wide variety of elements, many cherry-picked from HTML.  This page describes how to create these elements in a slot.  See the Elements section of the manual for more on how to modify and use these elements after they have been placed.

=== animate(fps) { |frame| ... } » Shoes::Animation ===

Starts an animation timer, which runs parallel to the rest of the app.  The `fps` is a number, the frames per seconds.  This number dictates how many times per second the attached block will be called.

The block is given a `frame` number.  Starting with zero, the `frame` number tells the block how many frames of the animation have been shown.

{{{
 #!ruby
 Shoes.app do
   @counter = para "STARTING"
   animate(24) do |frame|
     @counter.replace "FRAME #{frame}"
   end
 end
}}}

The above animation is shown 24 times per second.  If no number is give, the `fps` defaults to 10.

=== background(pattern) » Shoes::Background ===

Draws a Background element with a specific color (or pattern.)  Patterns can be colors, gradients or images.  Colors and images will tile across the background.  Gradients stretch to fill the background.

'''PLEASE NOTE:''' Backgrounds are actual elements, not styles.  HTML treats backgrounds like styles.  Which means every box can only have one background.  Shoes layers background elements.

{{{
 #!ruby
 Shoes.app do
   background black
   background white, :width => 50
 end
}}}

The above example paints two backgrounds.  First, a black background is painted over the entire app's surface area.  Then a 50 pixel white stripe is painted along the left side.

=== banner(text) » Shoes::Banner ===

Creates a Banner text block.  Shoes automatically styles this text to 48 pixels high.

=== border(text, :strokewidth => a number) » Shoes::Border ===

Draws a Border element using a specific color (or pattern.)  Patterns can be colors, gradients or images.  Colors and images will tile across the border.  Gradients stretch to fill the border.

'''PLEASE NOTE:''' Like Backgrounds, Borders are actual elements, not styles.  HTML treats backgrounds and borders like styles.  Which means every box can only have one borders.  Shoes layers border and background elements, along with text blocks, images, and everything else.

=== button(text) { ... } » Shoes::Button ===

Adds a push button with the message `text` written across its surface.  An optional block can be attached, which is called if the button is pressed.

=== caption(text) » Shoes::Caption ===

Creates a Caption text block.  Shoes styles this text to 14 pixels high.

=== check() » Shoes::Check ===

Adds a check box.

=== code(text) » Shoes::Code ===

Create a Code text fragment.  This text defaults to a monospaced font.

=== del(text) » Shoes::Del ===

Creates a Del text fragment (short for "deleted") which defaults to text with a single strikethrough in its middle.

=== edit_box(text) » Shoes::EditBox ===

Adds a large, multi-line textarea to this slot.  The `text` is optional and should be a string that will start out the box.  An optional block can be attached here which is called any type the user changes the text in the box.

{{{
 #!ruby
 Shoes.app do
   edit_box
   edit_box "HORRAY EDIT ME"
   edit_box "small one", :width => 100, :height => 160
 end
}}}

=== edit_line(text) » Shoes::EditLine ===

Adds a single-line text box to this slot.  The `text` is optional and should be a string that will start out the box.  An optional block can be attached here which is called any type the user changes the text in the box.

=== em(text) » Shoes::Em ===

Creates an Em text fragment (short for "emphasized") which, by default, is styled with italics.

=== every(seconds) { |count| ... } » Shoes::Every ===

A timer similar to the `animation` method, but much slower.  This timer fires a given number of seconds, running the block attached.  So, for example, if you need to check a web site every five minutes, you'd call `every(300)` with a block containing the code to actually ping the web site.

=== image(path) » Shoes::Image ===

Creates an Image element for displaying a picture.  PNG, JPEG and GIF formats are allowed.

=== imagesize(path) » [width, height] ===

Quickly grab the width and height of an image.  The image won't be loaded into the cache or displayed.

=== ins(text) » Shoes::Ins ===

Creates an Ins text fragment (short for "inserted") which Shoes styles with a single underline.

=== inscription(text) » Shoes::Inscription ===

Creates an Inscription text block.  Shoes styles this text at 10 pixels high.

=== link(text, :click => proc or string) » Shoes::Link ===

Creates a Link text block, which Shoes styles with a single underline and colors with a #06E (blue) colored stroke.

The default LinkHover style is also single-underlined with a #039 (dark blue) stroke.

=== list_box(:items => [strings, ...]) » Shoes::ListBox ===

Adds a drop-down list box containing entries for everything in the `items` array.  An optional block may be attached, which is called if anything in the box becomes selected by the user.

{{{
 #!ruby
 Shoes.app do
   stack :margin => 10 do
     para "Pick a card:"
     list_box :items => ["Jack", "Ace", "Joker"]
   end
 end
}}}

Call `ListBox#text` to get the selected string.  See the `ListBox` section under `Native` controls for more help.

=== progress() » Shoes::Progress ===

Adds a progress bar.

=== para(text) » Shoes::Para ===

Create a Para text block (short for "paragraph") which Shoes styles at 12 pixels high.

=== radio() » Shoes::Radio ===

Adds a radio button.

=== strong(text) » Shoes::Strong ===

Creates a Strong text fragment, styled in bold by default.

=== sub(text) » Shoes::Sub ===

Creates a Sub text fragment (short for "subscript") which defaults to lowering the text by 10 pixels and styling it in an x-small font.

=== subtitle(text) » Shoes::Subtitle ===

Creates a Subtitle text block.  Shoes styles this text to 26 pixels high.

=== sup(text) » Shoes::Sup ===

Creates a Sup text fragment (short for "superscript") which defaults to raising the text by 10 pixels and styling it in an x-small font.

=== tagline(text) » Shoes::Tagline ===

Creates a Tagline text block.  Shoes styles this text to 18 pixels high.

=== timer(seconds) { ... } » Shoes::Timer ===

A one-shot timer.  If you want to schedule to run some code in a few seconds (or minutes, hours) you can attach the code as a block here.

To display an alert box five seconds from now:

{{{
 #!ruby
 Shoes.app do
   timer(5) do
     alert("Your five seconds are up.")
   end
 end
}}}

=== title(text) » Shoes::Title ===

Creates a Title text block.  Shoes styles these elements to 34 pixels high.

=== video(path or url) » Shoes::Video ===

Embeds a movie in this slot.

== Manipulation Blocks ==

The manipulation methods below make quick work of shifting around slots and inserting new elements.

=== append() { ... } » self ===

Adds elements to the end of a slot.

{{{
 #!ruby
 @slot.append do
   title "Breaking News"
   tagline "Astronauts arrested for space shuttle DUI."
 end
}}}

The `title` and `tagline` elements will be added to the end of the `@slot`.

=== after(element) { ... } » self ===

Adds elements to a specific place in a slot, just after the `element` which is a child of the slot.

=== before(element) { ... } » self ===

Adds elements to a specific place in a slot, just before the `element` which is a child of the slot.

=== clear() » self ===

Empties the slot of any elements, timers and nested slots.  This is effectively identical to looping through
the contents of the slot and calling each element's `remove` method.

=== clear() { ... } » self ===

The clear method also takes an optional block.  The block will be used to replace the contents of the slot.

{{{
 #!ruby
 @slot = stack { para "Old text" }
 @slot.clear { para "Brand new text" }
}}}

In this example, the "Old text" paragraph will be cleared out, replaced by the "Brand new text" paragraph.

=== prepend() { ... } » self ===

Adds elements to the beginning of a slot.

{{{
 #!ruby
 @slot.append do
   para "Your car is ready."
 end
}}}

The `para` element is added to the beginning of the `@slot`.

== Styles of a Slot ==

Like any other element, slots can be styled and customized when they are created.

To set the width of a stack to 150 pixels:

{{{
 #!ruby
 stack(:width => 150) { para "Now that's precision." }
}}}

Each style setting also has a method, which can be used to grab that particular setting.  (So,
like, the `width` method returns the width of the slot in pixels.)

=== gutter() » a number ===

The size of the scrollbar area.  When Shoes needs to show a scrollbar, the scrollbar may end up covering up some elements that touch the edge of the window.  The `gutter` tells you how many pixels to expect the scrollbar to cover.

This is commonly used to pad elements on the right, like so:

{{{
 #!ruby
 stack :margin_right => 20 + gutter do
   para "Insert fat and ratified declaration of
     independence here..."
 end
}}}

=== height() » a number ===

The vertical size of the viewable slot in pixels.  So, if this is a scrolling slot, you'll need to use `scroll_top()` to get the full size of the slot.

=== scroll() » true or false ===

Is this slot allowed to show a scrollbar?  True or false.  The scrollbar will only appear if
the height of the slot is also fixed.

=== scroll_height() » a number ===

The vertical size of the full slot, including any of it which is hidden by scrolling.

=== scroll_max() » a number ===

The top coordinate which this slot can be scrolled down to.  The top coordinate of a scroll bar is always zero.  The bottom coordinate is the full height of the slot minus one page of scrolling.  This bottom coordinate is what `scroll_max` returns.

This is basically a shortcut for writing `slot.scroll_height - slot.height`.

To scroll to the bottom of a slot, use `slot.scroll_top = slot.scroll_max`.

=== scroll_top() » a number ===

The top coordinate which this slot is scrolled down to.  So, if the slot is scrolled down twenty pixels, this method will return `20`.

=== scroll_top = a number ===

Scrolls the slot to a certain coordinate.  This must be between zero and `scroll_max`.

=== style(styles) » styles ===

Alter the slot using a hash of style settings.  Any of the methods on this page (aside from this method, of course) can be used as a style setting.  So, for example, there is a `width` method, thus there is also a `width` style.

{{{
 #!ruby
 Shoes.app do
   @s = stack
   @s.style(:width => 400)
 end
}}}

=== width() » a number ===

The horizontal size of the slot in pixels.

== Traversing the Page ==

You may find yourself needing to loop through the elements inside a slot.  Or maybe you need to
climb the page, looking for a stack that is the parent of an element.

On any element, you may call the `parent` method to get the slot directly above it.  And on slots,
you can call the `children` method to get all of the children.  (Some elements, such as text blocks,
have a `contents` method for getting their children.)

=== children() » an array of elements ===

Lists all elements in a slot.

=== parent() » a Shoes::Stack or Shoes::Flow ===

Gets the object for this element's container.

= Elements =

Ah, here's the stuff of Shoes.  An element can be as simple as an oval shape.  Or as complex as
a video stream.  You've encountered all of these elements before in the Slots section of the
manual.

Once an element is created, you will often still want to change it.  To move it or hide it or get
rid of it.  You'll use the element's class to do that sort of stuff.

So, for example, use the `image` method of a Slot to place a PNG on the screen. The `image` method
gives you back an Image object. Use the methods of the Image object to change things up.

== Image ==

An image is a picture in PNG, JPEG or GIF format.  Shoes can resize images or flow them in with text.

To create an image, use the `image` method in a slot:

{{{
 #!ruby
 flow do
   para "Nice, nice, very nice.  Busy, busy, busy."
   image "static/disheveled.gif"
  end
}}}

=== height() » a number ===

The vertical screen size of the image in pixels.  This is not the original size of the image.
If you have a 150x150 pixel image and you set the width to 50 pixels, this method will return
50.

=== width() » a number ===

The horizontal screen size of the image in pixels.

== Native ==

Shoes has seven native controls: the Button, the EditLine, the EditBox, the ListBox, the Progress meter, the Check box and the Radio.

== TextBlock ==

The TextBlock object represents a group of text organized as a single element.  A paragraph containing bolded text, for example.  A caption containing links and underlined text.

== Timers ==

Shoes contains two timer classes: the Animation class and the Timer class.

== Video ==

Shoes supports embedding of QuickTime, Flash video (FLV), DivX, Xvid and various other popular video formats.  This is all thanks to VideoLAN and ffmpeg, two sensational open source libraries.  Use the `video` method on a slot to setup a Shoes::Video object.

In addition to video formats, some audio formats are also supported, such as MP3, WAV and Ogg Vorbis.

Video support is optional in Shoes and some builds do not support video.  For example, video support is unavailable for PowerPC.  When you download Shoes, the build for your platform will be marked `novideo` in the filename if no video support is available.

=== hide() » self ===

Hides the video.  If already playing, the video will continue to play.  This just turns off display of the video.  One possible use of this method is to collapse the video area when it is playing an audio file, such as an MP3.

=== length() » a number ===

The full length of the video in milliseconds.  Returns nil if the video is not yet loaded.

=== move(x, y) » self ===

Moves the video to specific coordinates, the (x, y) being the upper left hand corner of the video.

=== pause() » self ===

Pauses the video, if it is playing.

=== playing?() » true of false ===

Returns true if the video is currently playing.  Or, false if the video is paused or stopped.

=== play() » self ===

Starts playing the video, if it isn't already playing.  If already playing, the video is restarted from the beginning.

=== position() » a decimal ===

The position of the video as a decimanl number (a Float) between the beginning (0.0) and the end (1.0).  For instance, a Float value of 0.5 indicates the halfway point of the video.

=== position = a decimal ===

Sets the position of the video using a Float value.  To move the video to its 25% position: `@video.position = 0.25`.

=== remove() » self ===

Removes the video from its slot.  This will stop the video as well.

=== show() » self ===

Reveals the video, if it has been hidden by the `hide()` method.

=== stop() » self ===

Stops the video, if it is playing.

=== time() » a number ===

The time position of the video in milliseconds.  So, if the video is 10 seconds into play, this method would return the number 10000.

=== time = a number ===

Set the position of the video to a time in milliseconds.

=== toggle() » self ===

Toggles the visibility of the video.  If the video can be seen, then `hide` is called.  Otherwise, `show` is called.

END
