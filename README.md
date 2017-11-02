# THIS REPO IS NO LONGER ACTIVE!

Looking for Shoes 3 support (CRuby-based)? Try https://github.com/shoes3/shoes3

Looking for Shoes 4 (JRuby)? Try https://github.com/shoes/shoes4

<pre>
    ((( |||_| ///\ [[[_ (((
     ))) || |  \\/  [[_  )))
  an artsy any-platform app kit
      http://shoesrb.com
</pre>

# About Shoes

Shoes is the best little DSL for cross-platform GUI programming there is. It feels like real Ruby, rather than just another C++ library wrapper. If Gtk or wxWidgets is Rails, Shoes is Sinatra.

# Let me tell you a story about Shoes

  Way way back in the day, there was a guy named \_why. He created a project known as [Hackety Hack](http://hackety-hack.com) to teach programming to everyone. In order to reach all corners of the earth, \_why decided to make Hackety Hack work on Windows, Mac OS X, and Linux. This was a lot of work, and so \_why decided to share his toolkit with the world. Thus, Shoes was born.

Everybody loved Shoes and many apps were made. But, one day, \_why left. In his memory, Team Shoes assembled, and carried on making Shoes. They released Shoes 3 in late summer 2010.

# Attention: Development moved to [shoes4](https://github.com/shoes/shoes4)

Due to various issues (including compilation/release, stability) with this shoes implementation development moved on to [shoes4](https://github.com/shoes/shoes4). Shoes4 is a complete all Ruby rewrite of shoes. It's goal is to be as close to 100% backwards compatible as it can get e.g. it implements the same DSL.

So what does that mean? Well we accept pull requests but are not actively developing/releasing this shoes version. Our development efforts are now concentrated on shoes4, which is shaping up to become the new default implementation. It is already quite far and a lot of things already work.

# So what do these Shoes look like?

Here's a little Shoes app. It's a stopwatch!

```ruby
Shoes.app height: 150, width: 250 do
  background rgb(240, 250, 208)
  stack margin: 10 do
    button "Start" do
      @time = Time.now
      @label.replace "Stop watch started at #@time"
    end
    button "Stop" do
      @label.replace "Stopped, ", strong("#{Time.now - @time}"), " seconds elapsed."
    end
    @label = para "Press ", strong("start"), " to begin timing."
  end
end
```

Here's what it looks like:

![shoes timer](https://github.com/shoes/shoes/raw/develop/manual-snapshots/simple-timer.png)

Pretty simple! For more samples, the manual, and a free book, check out [the Shoes website](http://shoesrb.com/).

# Using Shoes

If you'd like to use Shoes to develop some apps... awesome! It's super easy: Just go to the [downloads page on the Shoes website](http://shoesrb.com/downloads) and download a copy of Shoes for your platform. Mac OSX, Windows, and Linux supported!

After you install Shoes, run it! You'll get a window like this:

![shoes main window](https://github.com/shoes/shoes/raw/develop/static/shoes_main_window.png)

You can then open any .rb file with Shoes code inside by choosing "Open an App." It'll open it up and run it, right away.

Once you're happy with your app, you can choose "Package an App" to wrap up your app as a .exe, .app, or a .run. Then you can share it with someone without a pair of Shoes to call their own.

# Making your own Shoes

You can make your own pair of Shoes with a little bit of elbow grease. Since there are different instructions on each platform, we've got a page up on the [Shoes development wiki](http://github.com/shoes/shoes/wiki) about it. It's [right here](https://github.com/shoes/shoes/wiki/Building-Shoes).

# Shoes Around the Web

If you want to keep up to date with what's going on with Shoes, you can find us in various places:

* [Official Shoes Site](http://shoesrb.com/)
* [Source Code @ GitHub](http://github.com/shoes/shoes)
* [Issue tracker @ GitHub](http://github.com/shoes/shoes/issues)
* [Mailing List](http://librelist.com/browser/shoes/) (send an email to shoes@librelist.com to join)
* [Twitter account](http://twitter.com/shoooesrb)
* [Facebook page](http://www.facebook.com/pages/Shoes/132605040125019)
* IRC room on Freenode, #shoes

# Helping out with Shoes

So you'd like to lend a helping hand, eh? Great! We'd love to have you. To submit a patch to Shoes, just fork us, and send a pull request.

If you don't have any ideas yourself, take a look at the [Issue tracker](http://github.com/shoes/shoes/issues) and see if anything strikes your fancy. If you need help working on something, don't be afraid to post to the mailing list about it!

Be sure to [peer into the Shoes Wiki](https://github.com/shoes/shoes/wiki) for instructions on how to get the source code to build, and to learn more knowledge that will come in handy if you want to help out!

If you're not a programmer, you can help Shoes by talking about it! Blog posts, tweets, tell your neighbors, call your grandma, whatever! Share Shoes with everyone!
