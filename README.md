<pre>
    ((( |||_| ///\ [[[_ (((
     ))) || |  \\/  [[_  )))
  an artsy any-platform app kit
      http://shoesrb.com
</pre>

## About

Shoes is the best little GUI toolkit. It takes advantage of Ruby's own
strengths to make writing applications easy and fun!

Shoes lets you write apps in Ruby that run on Mac OSX, Linux, and Windows. Like this:

``` ruby
Shoes.app :height => 150, :width => 250 do
	background rgb(240, 250, 208)

	stack :margin => 10 do
		button "Start" do
			@time = Time.now
			@label.replace "Stop watch started at #{@time}"
		end

		button "Stop" do
			@label.replace "Stopped, ", strong("#{Time.now - @time}"), " seconds elapsed."
		end

		@label = para "Press ", strong("start"), " to begin timing."
	end
end
```

If you ran that app with Shoes, you'd see this:

![shoes timer](https://github.com/shoes/shoes/raw/develop/manual-snapshots/simple-timer.png)

Super simple!

## Getting Shoes

You can get your own pair of Shoes by swinging by the [Shoes download page](http://shoesrb.com/downloads).
You can use it to run your own Shoes scripts.

If you're on Linux, your best bet is to [build Shoes from source](#), which is
actually pretty easy. Don't despair!

## Using Shoes

There's three steps to any Shoes project: learn about Shoes, write your app,
package your app.

### Learn

Of course, it's hard to know what you can do with Shoes without reading more!
There are two great resources to help you learn: [the manual](#) and [Nobody
Knows Shoes](#), a book written by Shoes' original author.

You can also check out the samples provided in the samples directory, and check
out even more of them in the [shoes-contrib project](#).

### Write

Next, you'll want to actually create your app. You can do this in your normal
text editor, just make regular old `.rb` files. Then open them with Shoes,
and try them out!

Don't forget about the Shoes console to help you diagnose errors, you can
bring it up with `control-/`, `command-/`, or `alt-/` depending on your
platform.

### Package

Finally, you can make your app into a .exe, .run, or .app file, depending on
which platform you're on. There are two kinds of packaging: from inside Shoes,
and from the command line.

#### Inside Shoes

Shoes has a 'package an app' command. It'll bring up a window where you can
make an executable. Just follow the steps!

You can also build a `.shy`, which is a tiny version of your app. Your user
will have to have Shoes installed to use the `.shy`, though.

#### Command line

This is how [Hackety Hack](http://hackety.com) gets created. However, it's
not well documented at all, and so is kinda a secret. You can build apps
with `$ APP=/path/to/app rake`, but it needs certain things to be in
certain places. We're working on getting documentation done for this.

## Get Help

If you'd like help with Shoes, the first stop is the [shoes mailing
list](mailto:shoes@librelist.com). Plenty of helpful people await!

Other than that, you can drop by #shoes on Freenode, and some people
hang out there. It's not super active, but sometimes it is!

## About Development of Shoes

All development is done on [GitHub](https://github.com/shoes/shoes). Nice and
easy. That's also where the [issues](https://github.com/shoes/shoes/issues)
live.

We talk about Shoes development on the [mailing list](http://librelist.com/browser/shoes/). You can send an email to shoes@librelist.com to join.

We post updates and links relevant to Shoes on our [Twitter
account](http://twitter.com/shoooesrb). It's not super high traffic, we try
to keep it strictly relevant!

## Contributing

Contributing a patch to Shoes is easy, just send us a pull request!

If you're not sure what you want to add or fix, try checking out the
[issues](http://github.com/shoes/shoes/issues) for a list of problems
other users have found.

We can always use the following, too:

* Code review by people fluent in C.
* People with good knowledge of Windows.
* Anyone who wants to write some more documentation.

