                      
    ((( |||_| ///\ [[[_ (((
     ))) || |  \\/  [[_  )))
    an artsy any-platform app kit <http://github.com/shoes3/shoes>
    
  The source describe below is at <https://github.com/shoes3/shoes> and
  you can follow my blog at <http://walkabout.mvmanila.com>

//////////////////////////////////////////////////////////////////////////

This is Shoes 3.2 (Federales). Earlier releases were Policeman (3.0 and 3.1)
and Raisins. There is a Shoes 4 under development. For historical reasons (the 
color of the icon in Raisins), it's also called Red Shoes. Red Shoes is written
in C (and some Objective C on the Mac). Shoes 4 is written in Java. 

Federales is a maintenance release to keep Red Shoes current with modern 
Linux, OSX, and Windows distributions and newer versions of Ruby and ruby gems.
Federales doesn't really add any notable new features to Shoes except it works better

Let's start with the good news first.

  Federales uses Ruby 2.0.0 (or higher) and Rubygems 2.0.14 (or higher) 
  
  Linux users really can download a binary that works. They don't have to 
    build from source. If they do build from source, it's easier.
     
  Added a Raspberry Pi distribution (Raspbian)
  
  For Linux, it installs Shoes in your home directory (~/.shoes/federales) 
    and you get a menu selection. You can start from the commandline if you
    like, of course.
    
  Gem handling is greatly improved.
    Gem.setup may not be not required. If used, it works better.
    
  Windows 7+ version works much, much better. Serveral nasty bugs have
    been fixed and the installer is less offensive.
    
  OSX Mavericks and Yosemite works. Snow Leopard (10.6) is possible.
  
  Includes a Shoes Cobbler app to clear your image cache and manage your Gems 
    
  Packaging works again! 
  
    From all 5 platforms to all 5 platforms.
    No longer tied to a dead website and you can change the website used
    for packaging and downloading without building Shoes from source.

What doesn't work.

  Link and Linkhover have to be replaced with Shoes::Link and Shoes::Linkhover
  
  The json gem is not included. Ruby provides it.
    
Important Concepts

  Ruby 1.9.3 brought a feature to ruby that made it very difficult to
  distrubute Shoes 3.0 or 3.1 or package a script with shoes. Shoes 3.2
  has two variants: 
  
    Tight Shoes - a sandboxed app you download. And send around.
    Loose Shoes - built from source and deeply tied to your system. You cannot
      distribute a Loose Shoes and expect it to work.
      
  You can use either Loose or Tight to Package but the package will always 
    use a Tight Shoes
    
  Loose Shoes (that you compiled) will use your existing ruby gems. Since you have enough skills
  to install ruby and the dependecies to build Shoes, you can handle gem
  version mismatches. Just use your normal ruby 'gem' command line skills to
  install the gem and Shoes will use them. Something for the hardcore to love.
  
  Say you have a 'Tight' Shoes, and maybe you'd like to have a Get Out Of Jail
  card, too? There is a way in Federales. Run the Shoes Cobbler from the splash
  screen or 'shoes -c' from the commandline. You can tell Shoes where those 
  other Ruby Gems are located. You'll have to have the compilers and build 
  tools installed  (Devkit for Windows - 32 bit version, or Xcode command line tool for OSX or
  whatever your Linux distribution uses to install gcc/make/autoconf.
  
  Perhaps you're thinking, "If I install all that stuff then I could just
  build Shoes from source!" You could but it won't as easy as you hope. 
  
I highly recommend you run the samples/simple-info.rb script. It'll tell you
what kind of Shoes 3.2 you have and the gem directories it knows about and 
has used. It's also a simple script to study. shoes/cobbler.rb also has clues
about how the jailbreak works and what you can do.


Remember, no one is happy if the Shoes don't fit.
  
  

