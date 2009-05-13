# -*- encoding: utf-8 -*-
#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
ARGV.delete_if { |x| x =~ /-psn_/ }

require 'open-uri'
require 'optparse'
require 'resolv-replace' if RUBY_PLATFORM =~ /win/
require 'shoes/inspect'
require 'shoes/cache'
if Object.const_defined? :Shoes
  require 'shoes/image'
end
 
class Range 
  def rand 
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv) 
  end 
end

unless Time.respond_to? :today
  def Time.today
    t = Time.now
    t - (t.to_i % 86400)
  end
end

class Shoes
  RELEASES = %w[Curious Raisins Policeman]

  NotFound = proc do
    para "404 NOT FOUND, GUYS!"
  end
 
  class << self; attr_accessor :locale, :language end
  @locale = ENV["SHOES_LANG"] || ENV["LC_MESSAGES"] || ENV["LC_ALL"] || ENV["LANG"] || "C"
  @language = @locale[/^(\w{2})_/, 1] || "en"

  @mounts = []

  OPTS = OptionParser.new do |opts|
    opts.banner = "Usage: shoes [options] (app.rb or app.shy)"
    
    opts.on("-m", "--manual",
            "Open the built-in manual.") do
      show_manual
    end

    opts.on("-p", "--package",
            "Package a Shoes app for Windows, OS X and Linux.") do |s|
      make_pack
    end

    opts.on("-g", "--gem",
            "Passes commands to RubyGems.") do
      require 'shoes/setup'
      require 'rubygems/gem_runner'
      Gem::GemRunner.new.run(ARGV)
      raise SystemExit, ""
    end

    opts.on("--manual-html DIRECTORY", "Saves the manual to a directory as HTML.") do |dir|
      manual_as :html, dir
      raise SystemExit, "HTML manual in: #{dir}"
    end

    opts.on("--install MODE SRC DEST", "Installs a file.") do |mode|
      src, dest = ARGV
      FileUtils.install src, dest, :mode => mode.to_i(8), :preserve => true
      raise SystemExit, ""
    end

    opts.on_tail("-v", "--version", "Display the version info.") do
      raise SystemExit, File.read("#{DIR}/VERSION.txt").strip
    end

    opts.on_tail("-h", "--help", "Show this message") do
      raise SystemExit, opts.to_s
    end
  end

  class SettingUp < StandardError; end

  @setups = {}

  def self.setup &blk
    require 'shoes/setup'
    line = caller[0]
    return if @setups[line]
    script = line[/^(.+?):/, 1]
    set = Shoes::Setup.new(script, &blk)
    @setups[line] = true
    unless set.no_steps?
      raise SettingUp
    end
  end

  def self.show_selector
    fname = ask_open_file
    Shoes.visit(fname) if fname
  end

  def self.splash
    font "#{DIR}/fonts/Lacuna.ttf"
    Shoes.app :width => 400, :height => 300, :resizable => false do  
      style(Para, :align => "center", :weight => "bold", :font => "Lacuna Regular", :size => 13)
      style(Link, :stroke => yellow, :underline => nil)
      style(LinkHover, :stroke => yellow, :fill => nil)

      x1 = 77; y1 = 122
      x2 = 148; y2 = -122
      x3 = 245; y3 = 0

      nofill
      strokewidth 40.0

      @waves = stack :top => 0, :left => 0

      stack :margin => 22 do
        para "Welcome to", :stroke => "#DFA", :margin => 0
        para "SHOES", :size => 48, :stroke => "#DFA", :margin_top => 0
        stack do
          background black(0.2), :curve => 8
          para link("Open an App.") { Shoes.show_selector and close }, :margin => 12, :margin_bottom => 4
          para link("Read the Manual.") { Shoes.show_manual and close }, :margin => 12 
        end
        inscription "Alt-Slash opens the console.", :stroke => "#DFA", :align => "center"
      end

      animate(10) do |ani|
        a = Math.sin(ani * 0.02) * 20
        @waves.clear do
          background white
          y = -30
          16.times do |i|
            shape do
              move_to x = (-300 - (i*(a*0.8))), y
              c = (a + 14) * 0.01
              stroke rgb(i * 0.06, c + 0.1, 0.1, 1.0 - (ani * 0.0003))
              4.times do
                curve_to x1 + x, (y1-(i*a)) + y, x2 + x, (y2+(i*a)) + y, x3 + x, y3 + y
                x += x3
              end
            end
            y += 30
          end
        end
      end
    end
  end

  def self.make_pack
    require 'shoes/pack'
    Shoes.app(:width => 500, :height => 380, :resizable => false, &PackMake)
  end

  def self.manual_p(str, path)
    str.gsub(/\n+\s*/, " ").
      gsub(/&/, '&amp;').gsub(/>/, '&gt;').gsub(/>/, '&lt;').gsub(/"/, '&quot;').
      gsub(/`(.+?)`/m, '<code>\1</code>').gsub(/\[\[BR\]\]/i, "<br />\n").
      gsub(/\^(.+?)\^/m, '\1').
      gsub(/'''(.+?)'''/m, '<strong>\1</strong>').gsub(/''(.+?)''/m, '<em>\1</em>').
      gsub(/\[\[(http:\/\/\S+?)\]\]/m, '<a href="\1" target="_new">\1</a>').
      gsub(/\[\[(http:\/\/\S+?) (.+?)\]\]/m, '<a href="\1" target="_new">\2</a>').
      gsub(/\[\[(\S+?)\]\]/m) do
        ms, mn = $1.split(".", 2)
        if mn
          '<a href="' + ms + '.html#' + mn + '">' + mn + '</a>'
        else
          '<a href="' + ms + '.html">' + ms + '</a>'
        end
      end.
      gsub(/\[\[(\S+?) (.+?)\]\]/m, '<a href="\1.html">\2</a>').
      gsub(/\!(\{[^}\n]+\})?([^!\n]+\.\w+)\!/) do
        x = "static/#$2"
        FileUtils.cp("#{DIR}/#{x}", "#{path}/#{x}") if File.exists? "#{DIR}/#{x}"
        '<img src="' + x + '" />'
      end
  end

  def self.manual_link(sect)
  end

  TITLES = {:title => :h1, :subtitle => :h2, :tagline => :h3, :caption => :h4}

  def self.manual_as format, *args
    require 'shoes/search'
    require 'shoes/help'

    case format
    when :shoes
      Shoes.app(:width => 720, :height => 640, &Shoes::Help)
    else
      extend Shoes::Manual
      man = self
      dir, = args
      FileUtils.mkdir_p File.join(dir, 'static')
      FileUtils.cp "static/shoes-icon.png", "#{dir}/static"
      %w[manual.css code_highlighter.js code_highlighter_ruby.js].
        each { |x| FileUtils.cp "static/#{x}", "#{dir}/static" }
      html_bits = proc do
        proc do |sym, text|
        case sym when :intro
          div.intro { p { self << man.manual_p(text, dir) } }
        when :code
          pre { code.rb text.gsub(/^\s*?\n/, '') }
        when :colors
          color_names = (Shoes::COLORS.keys*"\n").split("\n").sort
          color_names.each do |color|
            c = Shoes::COLORS[color.intern]
            f = c.dark? ? "white" : "black"
            div.color(:style => "background: #{c}; color: #{f}") { h3 color; p c }
          end
        when :index
          tree = man.class_tree
          shown = []
          i = 0
          index_p = proc do |k, subs|
            unless shown.include? k
              i += 1
              p "▸ #{k}", :style => "margin-left: #{20*i}px"
              subs.uniq.sort.each do |s|
                index_p[s, tree[s]]
              end if subs
              i -= 1
              shown << k
            end
          end
          tree.sort.each &index_p
        #   index_page
        when :list
          ul { text.each { |x| li { self << man.manual_p(x, dir) } } }
        else
          send(TITLES[sym] || :p) { self << man.manual_p(text, dir) }
        end
        end
      end

      docs = load_docs(Shoes::Manual::PATH)
      sections = docs.map { |x,| x }

      docn = 1
      docs.each do |title1, opt1|
        subsect = opt1['sections'].map { |x,| x }
        menu = sections.map do |x|
          [x, (subsect if x == title1)]
        end

        path1 = File.join(dir, title1.gsub(/\W/, ''))
        make_html("#{path1}.html", title1, menu) do
          h2 "The Shoes Manual"
          h1 title1
          man.wiki_tokens opt1['description'], true, &instance_eval(&html_bits)
          p.next { text "Next: "
            a opt1['sections'].first[1]['title'], :href => "#{opt1['sections'].first[0]}.html" }
        end

        optn = 1
        opt1['sections'].each do |title2, opt2|
          path2 = File.join(dir, title2)
          make_html("#{path2}.html", opt2['title'], menu) do
            h2 "The Shoes Manual"
            h1 opt2['title']
            man.wiki_tokens opt2['description'], true, &instance_eval(&html_bits)
            opt2['methods'].each do |title3, desc3|
              sig, val = title3.split(/\s+»\s+/, 2)
              aname = sig[/^[^(=]+=?/].gsub(/\s/, '').downcase
              a :name => aname
              div.method do
                a sig, :href => "##{aname}"
                text " » #{val}" if val
              end
              div.sample do
                man.wiki_tokens desc3, &instance_eval(&html_bits)
              end
            end
            if opt1['sections'][optn]
              p.next { text "Next: "
                a opt1['sections'][optn][1]['title'], :href => "#{opt1['sections'][optn][0]}.html" }
            elsif docs[docn]
              p.next { text "Next: "
                a docs[docn][0], :href => "#{docs[docn][0].gsub(/\W/, '')}.html" }
            end
            optn += 1
          end
        end

        docn += 1
      end
    end
  end

  def self.show_manual
    manual_as :shoes
  end

  def self.show_log
    require 'shoes/log'
    return if @log_app and Shoes.APPS.include? @log_app
    @log_app =
      Shoes.app do
        extend Shoes::LogWindow
        setup
      end
  end

  def self.mount(path, meth, &blk)
    @mounts << [path, meth || blk]
  end

  SHOES_URL_RE = %r!^@([^/]+)(.*)$! 

  def self.run(path)
    uri = URI(path)
    @mounts.each do |mpath, rout|
      m, *args = *path.match(/^#{mpath}$/)
      if m
        unless rout.is_a? Proc
          rout = rout[0].instance_method(rout[1])
        end
        return [rout, args]
      end
    end
    case uri.path when "/"
      [nil]
    when SHOES_URL_RE
      [proc { eval(URI("http://#$1:53045#$2").read) }]
    else
      [NotFound]
    end
  end

  def self.args!
    if RUBY_PLATFORM !~ /darwin/ and ARGV.empty?
      Shoes.splash
    end
    OPTS.parse! ARGV
    ARGV[0] or true
  end

  def self.uri(str)
    if str =~ SHOES_URL_RE
      URI("http://#$1:53045#$2")
    else
      URI(str) rescue nil
    end
  end

  def self.visit(path)
    uri = Shoes.uri(path)

    case uri
    when URI::HTTP
      str = uri.read
      if str !~ /Shoes\.app/
        Shoes.app do
          eval(uri.read)
        end
      else
        eval(uri.read)
      end
    else
      path = File.expand_path(path.gsub(/\\/, "/"))
      if path =~ /\.shy$/
        require 'shoes/shy'
        base = File.basename(path, ".shy")
        tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
        shy = Shy.x(path, tmpdir)
        Dir.chdir(tmpdir)
        Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
        path = shy.launch
      else
        Dir.chdir(File.dirname(path))
        path = File.basename(path)
      end

      $0.replace path

      code = read_file(path)
      eval(code, TOPLEVEL_BINDING, path)
    end
  rescue SettingUp
  rescue Object => e
    error(e)
    show_log
  end

  def self.read_file path
    if RUBY_VERSION =~ /^1\.9/
      File.open(path, 'r:utf-8') { |f| f.read }
    else
      File.read(path)
    end
  end

  def self.url(path, meth)
    Shoes.mount(path, [self, meth])
  end

  module Basic
    def tween opts, &blk
      opts = opts.dup

      if opts[:upward]
        opts[:top] = self.top - opts.delete(:upward)
      elsif opts[:downward]
        opts[:top] = self.top + opts.delete(:downward)
      end
      
      if opts[:sideways]
        opts[:left] = self.left + opts.delete(:sideways)
      end
      
      @TWEEN.remove if @TWEEN
      @TWEEN = parent.animate(opts[:speed] || 20) do

        # figure out a coordinate halfway between here and there
        cont = opts.select do |k, v|
          if self.respond_to? k
            n, o = v, self.send(k)
            if n != o
              n = o + ((n - o) / 2)
              n = v if o == n
              self.send("#{k}=", n)
            end
            self.style[k] != v
          end
        end

        # if we're there, get rid of the animation
        if cont.empty?
          @TWEEN.remove
          @TWEEN = nil
          blk.call if blk
        end
      end
    end
  end

  # complete list of styles
  BASIC_S = [:left, :top, :right, :bottom, :width, :height, :attach, :hidden,
             :displace_left, :displace_top, :margin, :margin_left, :margin_top,
             :margin_right, :margin_bottom]
  TEXT_S  = [:strikecolor, :undercolor, :font, :size, :family, :weight,
             :rise, :kerning, :emphasis, :strikethrough, :stretch, :underline,
             :variant]
  MOUSE_S = [:click, :motion, :release, :hover, :leave]
  COLOR_S = [:stroke, :fill]

  {Background => [:angle, :radius, :curve, *BASIC_S],
   Border     => [:angle, :radius, :curve, :strokewidth, *BASIC_S],
   Canvas     => [:scroll, :start, :finish, :keypress, *(MOUSE_S|BASIC_S)],
   Check      => [:click, :checked, *BASIC_S],
   Radio      => [:click, :checked, :group, *BASIC_S],
   EditLine   => [:change, :secret, :text, *BASIC_S],
   EditBox    => [:change, :text, *BASIC_S],
   Effect     => [:radius, :distance, :inner, *(COLOR_S|BASIC_S)],
   Image      => MOUSE_S|BASIC_S,
   ListBox    => [:change, :items, :choose, *BASIC_S],
   # Pattern    => [:angle, :radius, *BASIC_S],
   Progress   => BASIC_S,
   Shape      => COLOR_S|MOUSE_S|BASIC_S,
   TextBlock  => [:justify, :align, :leading, *(COLOR_S|MOUSE_S|TEXT_S|BASIC_S)],
   Text       => COLOR_S|MOUSE_S|TEXT_S|BASIC_S}.
  each do |klass, styles|
    klass.class_eval do
      include Basic
      styles.each do |m|
        case m when *MOUSE_S
        else
          define_method(m) { style[m] } unless klass.method_defined? m
          define_method("#{m}=") { |v| style(m => v) } unless klass.method_defined? "#{m}="
        end
      end
    end
  end

  class Types::Widget
    @types = {}
    def self.inherited subc
      methc = subc.to_s[/(^|::)(\w+)$/, 2].
              gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
              gsub(/([a-z\d])([A-Z])/,'\1_\2').downcase
      @types[methc] = subc
      Shoes.class_eval %{
        def #{methc}(*a, &b)
          a.unshift Widget.instance_variable_get("@types")[#{methc.dump}]
          widget(*a, &b)
        end
      }
    end
  end
end

def window(*a, &b)
  Shoes.app(*a, &b)
end
