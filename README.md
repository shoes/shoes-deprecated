                      
    ((( |||_| ///\ [[[_ (((
     ))) || |  \\/  [[_  )))
    an artsy any-platform app kit <http://github.com/shoes/shoes>
    
  The source describe below is at <https://github.com/ccoupe/shoes> and
  you can follow my blog at <http://shoes.mvmanila.com>

//////////////////////////////////////////////////////////////////////////

This is Shoes 3.2 (Federales). Earlier releases were Policeman (3.0 and 3.1)
and Raisins. There is a Shoes 4 under development. For historical reasons (the 
color of the icon in Raisins) It's also called Red Shoes. Red Shoes is written
in C (and some Objective C on the Mac). Shoes 4 is written in Java. 

Federales is one mans attempt to keep Red Shoes current with modern Linux
distributions and newer versions of Ruby. Federales doesn't really add any new 
features to Shoes except it works better (Shoes 3.1 barely works)
As you'll see, Federales removes some things. 

Let's start with the good news first.

  Federales uses Ruby 2.0.0 (or higher) and Rubygems 2.0.14 (or higher) 
  Linux users really can download a binary that works. They don't have to 
    build from source. If they do build from source, it's easier. 
  Added a Raspberry Pi distribution (Raspbian)
  For Linux, it installs Shoes in your home directory (~/.shoes/federales) 
    and you get a menu selection. You can start from the commandline if you
    like, of course.
  Gem handling is greatly improved, at a cost - more on that below. 
    Gem.setup is not required. If used, it works better.
  Windows 7 version seems to work.
  OSX Mavericks (10.9.3) version is 'not completely terrible' as of 
    May 24, 2014
  Does not require Sqlite.
  Includes a Shoes Cobbler app to clear your image cache and manage your
    Shoes/gem interface - see below discussion on gems.
  

What doesn't work.
  Link and Linkhover have to be replaced with Shoes::Link and Shoes::Linkhover
  There are no built in Gems included. No Hpricot. No Sqlite. No json.
    Again - see below.
  Packaging an app to include Shoes with it does not work. It hasn't for
     years.  I'll let you know when it does. Be very patient. There is
     much to do first.
  Samples that use Sqllite or Hpricot. 
     
Gem Secret Sauce.
  If you build Shoes from source on Linux, you'll create a 'Loose' Shoes.
  If you download a Shoes 3.2 binary distribution, you'll get a 'Tight'
    shoes. Only gems in your ~/.shoe/+gem directory are allowed and you have
    to do the Shoes.setup in your code to get them and that may not work 
    as well as you hope. Just like Shoes 3.0 and 3.1
    
  Loose Shoes will use your existing ruby gems. Since you have enough skills
  to install ruby and the dependecies to build Shoes, you can handle gem
  version mismatches. Just use your normal ruby 'gem' command line skills to
  install the gem and Shoes will use them.  I call that a Jailbreak. 
  
  If you have a 'Tight' Shoes, and maybe you'd like to have a Get Out Of Jail
  card, too? There is a way in Federales. Run the Shoes Cobbler from the splash
  screen or 'shoes -c' from the commandline. You can tell Shoes where those 
  other Ruby Gems are located. You'll have to have the compilers and build 
  tools installed  (Devkit for Windows - 32 bit version, or Xcode for OSX or
  whatever your Linux distribution uses to install gcc/make/autoconf.
  
  Perhaps you're thinking, "If I install all that stuff then I could just
  build Shoes from source!" Correct! Shoe3.2 is a work in progress
  and Jailbreak is only a 'good idea', and not fully baked. Now that many gems
  include binary payloads for Windows, you may not need a developers setup.
  Or you might. I can't tell you what to do after you get out of jail. 
  
I highly recommend you run the samples/simple-info.rb script. It'll tell you
what kind of Shoes 3.2 you have and the gem directories it knows about and 
has used. It's also a simple script to study. shoes/cobbler.rb also has clues
about how the jailbreak works and what you can do .

It's all easy until it gets hard.
  
  

