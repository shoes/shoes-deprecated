# -*- encoding: utf-8 -*-
#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#

require_relative 'shoes/cache' # do First thing
ARGV.delete_if { |x| x =~ /-psn_/ }

# Probably don't need this
class Encoding
  %w(UTF_7 UTF_16BE UTF_16LE UTF_32BE UTF_32LE).each do |enc|
    eval "class #{enc};end" unless const_defined? enc.to_sym
  end
end

require 'open-uri'
require 'optparse'
require 'resolv-replace' if RUBY_PLATFORM =~ /win/
require_relative 'shoes/inspect'
require_relative 'shoes/image' if Object.const_defined? :Shoes

def Shoes.hook; end

# class Encoding
# %w[ASCII_8BIT UTF_16BE UTF_16LE UTF_32BE UTF_32LE US_ASCII].each do |ec|
#   eval "#{ec} = '#{ec.sub '_', '-'}'"
# end unless RUBY_PLATFORM =~ /linux/ or RUBY_PLATFORM =~ /darwin/ or RUBY_PLATFORM =~ /mingw/
# end

class Range
  def rand
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv)
  end
end

unless Time.respond_to? :today
  def Time.today
    t = Time.now
    t - (t.to_i % 86_400)
  end
end

class Shoes
  RELEASES = %w(Curious Raisins Policeman Federales Walkabout)

  NotFound = proc do
    para '404 NOT FOUND, GUYS!'
  end

  class << self; attr_accessor :locale, :language end
  @locale = ENV['SHOES_LANG'] || ENV['LC_MESSAGES'] || ENV['LC_ALL'] || ENV['LANG'] || 'C'
  @language = @locale[/^(\w{2})_/, 1] || 'en'

  @mounts = []

  OPTS = OptionParser.new do |opts|
    opts.banner = "Usage: shoes [options] (app.rb or app.shy)"

    opts.on('-d', '--debug', 'Debug Shoes script') do
      ENV['CMDLINE_DEBUG'] = true.to_s
    end

    opts.on("-m", "--manual",
            "Open the built-in manual.") do
      show_manual
    end

    opts.on("-w", "--console", "display console") do
      if ENV['console_loop']
        show_log # need something on the screen for Shoes- FIXME
        show_console
        require 'readline'
        require 'io/console'
        Thread.new do
          loop do
            #$stdout.write "prompt: "
            #ln = $stdin.gets
            #ln = $stdin.readline
            #Readline::vi_editing_mode
            ln = Readline::readline('> ', false)
            #ln = STDIN.cooked(&:gets)

            if ln.strip == 'quit'
              $stderr.write "really quit (y/n)"
              ans = $stdin.gets.strip
              exit if ans == 'y'
            end
            $stdout.puts "Shoes: #{ln}"
          end
        end
      else
        #ENV['SHOES_CONSOLE'] = true.to_s
        show_console
      end
    end

    opts.on("--old-package",
            "(Obsolete) Package a Shoes app for Windows, OS X and Linux.") do |s|
      make_pack
    end

    opts.on("-c", "--cobbler",
            "Maintain Shoes installation") do |c|
      cobbler
    end

    opts.on("-p", "--package",
            "Package Shoes App (new)") do |c|
      app_package
    end
    
    opts.on('-g', '--gem',
            'Passes commands to RubyGems.') do
      require 'shoes/setup'
      require 'rubygems/gem_runner'
      Gem::GemRunner.new.run(ARGV)
      fail SystemExit, ''
    end

    opts.on('--manual-html DIRECTORY', 'Saves the manual to a directory as HTML.') do |dir|
      manual_as :html, dir
      fail SystemExit, "HTML manual in: #{dir}"
    end

    opts.on('--install MODE SRC DEST', 'Installs a file.') do |mode|
      src, dest = ARGV
      FileUtils.install src, dest, mode: mode.to_i(8), preserve: true
      fail SystemExit, ''
    end

    opts.on('--nolayered', 'No WS_EX_LAYERED style option.') do
      $NOLAYERED = 1
      Shoes.args!
    end
    
    opts.on('-f', '--file', 'path to script [OSX packaging uses this]') do
      #puts "-f ARGV: #{ARGV}"
    end
    
    opts.on_tail('-v', '--version', 'Display the version info.') do
      # raise SystemExit, File.read("#{DIR}/VERSION.txt").strip
      # str = "Shoes #{Shoes::VERSION_NAME} r#{Shoes::VERSION_REVISION} #{Shoes::VERSION_DATE} #{RUBY_PLATFORM} #{RUBY_VERSION}"
      # $stderr.puts str
      fail SystemExit, "Shoes #{Shoes::VERSION_NAME} #{Shoes::VERSION_NUMBER} r#{Shoes::VERSION_REVISION} #{RUBY_PLATFORM} #{RUBY_VERSION}"
    end

    opts.on_tail('-h', '--help', 'Show this message') do
      fail SystemExit, opts.to_s
    end
  end

  class SettingUp < StandardError; end

  @setups = {}

  def self.setup(&blk)
    require 'shoes/setup'
    line = caller[0]
    return if @setups[line]
    script = line[/^(.+?):/, 1]
    set = Shoes::Setup.new(script, &blk)
    @setups[line] = true
    fail SettingUp unless set.no_steps?
  end

  def self.show_selector (debug = false)
    fname = ask_open_file
    Shoes.visit(fname, debug) if fname
  end

  def self.package_app
    require_relative 'shoes/shybuilder'
    fname = ask_open_file
    return false unless fname
    start_shy_builder fname
  end

  def self.splash
    font "#{DIR}/fonts/Lacuna.ttf"
    Shoes.app width: 598, height: 520, resizable: false do
      background "#{DIR}/static/splash.png"
      style(Para, align: 'center', weight: 'bold', font: 'Lacuna Regular', size: 13)
      style(Link, stroke: khaki, underline: nil)
      style(LinkHover, stroke: yellow, fill: nil)

      require 'shoes/search'
      require 'shoes/help'

      stack margin: 18 do
        para 'Welcome to', stroke: ivory, size: 18, margin: 0
        para 'SHOES', size: 24, stroke: ivory, margin: 0
        para Shoes::VERSION_NAME, stroke: ivory, size: 14, margin: 0, weight: 'bold'
        para "build #{Shoes::VERSION_NUMBER} r#{Shoes::VERSION_REVISION}", size: 14, stroke: ivory, margin_top: 0
        stack do
          background black(0.3), :curve => 8
          para link(strong("Open an App")) { Shoes.show_selector and close }, :margin => 10, :margin_bottom => 4
#         para link(strong("Debug an App")) { Shoes.show_selector true and close }, :margin => 10, :margin_bottom => 4
          para link(strong("Package my script (shy)")) { Shoes.package_app and close }, :margin => 10, :margin_bottom => 4
          para link(strong("Package an App with Shoes")) {Shoes.app_package and close }, :margin => 10, :margin_bottom => 4
#         para link("Obsolete: Package") { Shoes.make_pack and close }, :margin => 10, :margin_bottom => 4
          para link(strong("Read the Manual")) { Shoes.show_manual and close }, :margin => 10, :margin_bottom => 4
          para link(strong("Maintain Shoes")) {Shoes.cobbler and close}, :margin => 10
        end
        para 'Alt-Slash opens the console', stroke: '#00', align: 'center'
      end
    end
  end

  def self.cobbler
    require 'shoes/cobbler'
  end

  def self.app_package
    require 'shoes/app_package'
  end

  def self.make_pack
    #    require 'shoes/packgui'
    #    Shoes.app(:width => 500, :height => 480, :resizable => true, &Packshow)
    require 'shoes/pack'
    Shoes.app(width: 500, height: 480, resizable: true, &PackMake)
  end

  def self.manual_p(str, path)
    str.gsub(/\n+\s*/, ' ')
      .gsub(/&/, '&amp;').gsub(/>/, '&gt;').gsub(/>/, '&lt;').gsub(/"/, '&quot;')
      .gsub(/`(.+?)`/m, '<code>\1</code>').gsub(/\[\[BR\]\]/i, "<br />\n")
      .gsub(/\^(.+?)\^/m, '\1')
      .gsub(/'''(.+?)'''/m, '<strong>\1</strong>').gsub(/''(.+?)''/m, '<em>\1</em>')
      .gsub(/\[\[(http:\/\/\S+?)\]\]/m, '<a href="\1" target="_new">\1</a>')
      .gsub(/\[\[(http:\/\/\S+?) (.+?)\]\]/m, '<a href="\1" target="_new">\2</a>')
      .gsub(/\[\[(\S+?)\]\]/m) do
        ms, mn = Regexp.last_match(1).split('.', 2)
        if mn
          '<a href="' + ms + '.html#' + mn + '">' + mn + '</a>'
        else
          '<a href="' + ms + '.html">' + ms + '</a>'
        end
      end
      .gsub(/\[\[(\S+?) (.+?)\]\]/m, '<a href="\1.html">\2</a>')
      .gsub(/\!(\{[^}\n]+\})?([^!\n]+\.\w+)\!/) do
        x = "static/#{Regexp.last_match(2)}"
        FileUtils.cp("#{DIR}/#{x}", "#{path}/#{x}") if File.exist? "#{DIR}/#{x}"
        '<img src="' + x + '" />'
      end
  end

  def self.manual_link(_sect)
  end

  TITLES = { title: :h1, subtitle: :h2, tagline: :h3, caption: :h4 }

  def self.manual_as(format, *args)
    require 'shoes/search'
    require 'shoes/help'

    case format
    when :shoes
      Shoes.app(width: 720, height: 640, &Shoes::Help)
    else
      extend Shoes::Manual
      man = self
      dir, = args
      FileUtils.mkdir_p File.join(dir, 'static')
      FileUtils.cp "#{DIR}/static/shoes-icon.png", "#{dir}/static"
      %w(manual.css code_highlighter.js code_highlighter_ruby.js)
        .each { |x| FileUtils.cp "#{DIR}/static/#{x}", "#{dir}/static" }
      html_bits = proc do
        proc do |sym, text|
          case sym when :intro
                     div.intro { p { self << man.manual_p(text, dir) } }
          when :code
            pre { code.rb text.gsub(/^\s*?\n/, '') }
          when :colors
            color_names = (Shoes::COLORS.keys * "\n").split("\n").sort
            color_names.each do |color|
              c = Shoes::COLORS[color.intern]
              f = c.dark? ? 'white' : 'black'
              div.color(style: "background: #{c}; color: #{f}") { h3 color; p c }
            end
          when :index
            tree = man.class_tree
            shown = []
            i = 0
            index_p = proc do |k, subs|
              unless shown.include? k
                i += 1
                p "▸ #{k}", style: "margin-left: #{20 * i}px"
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
          when :samples
            folder = File.join DIR, 'samples'
            h = {}
            Dir.glob(File.join folder, '*').each do |file|
              if File.extname(file) == '.rb'
                key = File.basename(file).split('-')[0]
                h[key] ? h[key].push(file) : h[key] = [file]
              end
            end
            h.each do |k, v|
              p "<h4>#{k}</h4>"
              samples = []
              v.each do |file|
                sample = File.basename(file).split('-')[1..-1].join('-')[0..-4]
                samples << "<a href=\"http://github.com/shoes/shoes/raw/master/manual-snapshots/#{k}-#{sample}.png\">#{sample}</a>"
              end
              p samples.join ' '
            end
          else
            send(TITLES[sym] || :p) { self << man.manual_p(text, dir) }
          end
        end
      end

      docs = load_docs(Shoes::Manual.path)
      sections = docs.map { |x,| x }

      docn = 1
      docs.each do |title1, opt1|
        subsect = opt1['sections'].map { |x,| x }
        menu = sections.map do |x|
          [x, (subsect if x == title1)]
        end

        path1 = File.join(dir, title1.gsub(/\W/, ''))
        make_html("#{path1}.html", title1, menu) do
          h2 'The Shoes Manual'
          h1 title1
          man.wiki_tokens opt1['description'], true, &instance_eval(&html_bits)
          p.next do
            text 'Next: '
            a opt1['sections'].first[1]['title'], href: "#{opt1['sections'].first[0]}.html"
          end
        end

        optn = 1
        opt1['sections'].each do |title2, opt2|
          path2 = File.join(dir, title2)
          make_html("#{path2}.html", opt2['title'], menu) do
            h2 'The Shoes Manual'
            h1 opt2['title']
            man.wiki_tokens opt2['description'], true, &instance_eval(&html_bits)
            opt2['methods'].each do |title3, desc3|
              sig, val = title3.split(/\s+»\s+/, 2)
              aname = sig[/^[^(=]+=?/].gsub(/\s/, '').downcase
              a name: aname
              div.method do
                a sig, href: "##{aname}"
                text " » #{val}" if val
              end
              div.sample do
                man.wiki_tokens desc3, &instance_eval(&html_bits)
              end
            end
            if opt1['sections'][optn]
              p.next do
                text 'Next: '
                a opt1['sections'][optn][1]['title'], href: "#{opt1['sections'][optn][0]}.html"
              end
            elsif docs[docn]
              p.next do
                text 'Next: '
                a docs[docn][0], href: "#{docs[docn][0].gsub(/\W/, '')}.html"
              end
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

  def self.show_irb
    require 'shoes/irb'
    Shoes.irb
  end

  def self.remote_debug
    require "shoes/remote_debugger.rb"
    Shoes.rdb
  end

  def self.show_log
    require 'shoes/log'
    return if @log_app && Shoes.APPS.include?(@log_app)
    @log_app =
      Shoes.app do
        extend Shoes::LogWindow
        setup
      end
  end

  def self.mount(path, meth, &blk)
    @mounts << [path, meth || blk]
  end


  # SHOES_URL_RE = %r!^@([^/]+)(.*)$!
  SHOES_URL_RE = %r{^@([^/]+)(.*)$}

  def self.run(path)
    uri = URI(path)
    @mounts.each do |mpath, rout|
      m, *args = *path.match(/^#{mpath}$/)
      if m
        rout = rout[0].instance_method(rout[1]) unless rout.is_a? Proc
        # return [rout, args]
        return [rout, args, rout.owner] # requires change in app.c
      end
    end
    case uri.path when '/'
                    [nil]
    when SHOES_URL_RE
      [proc { eval(URI("http://#{Regexp.last_match(1)}:53045#{Regexp.last_match(2)}").read) }]
    else
      [NotFound]
    end
  end

  def self.args!
    Shoes.splash if RUBY_PLATFORM !~ /darwin/ && ARGV.empty?
    OPTS.parse! ARGV
    ARGV[0] || true
  end

  def self.uri(str)
    if str =~ SHOES_URL_RE
      URI("http://#{Regexp.last_match(1)}:53045#{Regexp.last_match(2)}")
    else
      URI(str) rescue nil
    end
  end

  def self.visit(path, debug=false)
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
      path = File.expand_path(path.gsub(/\\/, '/'))
      if path =~ /\.shy$/
        @shy = true
        require 'shoes/shy'
        base = File.basename(path, '.shy')
        #@tmpdir = tmpdir = '%s/shoes-%s.%d' % [Dir.tmpdir, base, $PROCESS_ID]
        @tmpdir = tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
        shy = Shy.x(path, tmpdir)
        Dir.chdir(tmpdir)
        # Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
        path = shy.launch
      else
        @shy = false
        Dir.chdir(File.dirname(path))
        path = File.basename(path)
      end
      if ENV['CMDLINE_DEBUG']
        require 'byebug'
        require 'byebug/runner'
        $PROGRAM_NAME = path
        Byebug.debug_load($PROGRAM_NAME, true) # this starts byebug loop
      elsif debug
        # spin up the console window and call the debugger with the path
        require 'shoes/debugger'
        @console_app =
          Shoes.app do
            extend Shoes::Debugger
            setup path
          end
      else
        $0.replace path
        code = read_file(path)
        eval(code, TOPLEVEL_BINDING, path)
      end
    end
  rescue SettingUp
  rescue Object => e
    error(e)
    show_log
  end

  def self.clean
    if @shy
      Dir.chdir() # do it from HOME 
      FileUtils.rm_rf(@tmpdir, secure: true)
    end
  end

  def self.read_file(path)
    if RUBY_VERSION =~ /^1\.9/ && !@shy
      # File.open(path, 'r:utf-8') { |f| f.read }
      IO.read(path).force_encoding('UTF-8')
    elsif RUBY_VERSION =~ /^2\.0/ && !@shy
      IO.read(path).force_encoding('UTF-8')
    else
      File.read(path)
    end
  end

  def self.url(path, meth)
    Shoes.mount(path, [self, meth])
  end

  module Basic
    def tween(opts, &blk)
      opts = opts.dup

      if opts[:upward]
        opts[:top] = top - opts.delete(:upward)
      elsif opts[:downward]
        opts[:top] = top + opts.delete(:downward)
      end

      if opts[:sideways]
        opts[:left] = self.left + opts.delete(:sideways)
      end

      @TWEEN.remove if @TWEEN
      @TWEEN = parent.animate(opts[:speed] || 20) do
        # figure out a coordinate halfway between here and there
        cont = opts.select do |k, v|
          if self.respond_to? k
            n = v
            o = send(k)
            if n != o
              n = o + ((n - o) / 2)
              n = v if o == n
              send("#{k}=", n)
            end
            style[k] != v
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
  KEY_S   = [:keydown, :keypress, :keyup]
  COLOR_S = [:stroke, :fill]

  { Background => [:angle, :radius, :curve, *BASIC_S],
    Border     => [:angle, :radius, :curve, :strokewidth, *BASIC_S],
    Canvas     => [:scroll, :start, :finish, *(KEY_S | MOUSE_S | BASIC_S)],
    Check      => [:click, :checked, *BASIC_S],
    Radio      => [:click, :checked, :group, *BASIC_S],
    EditLine   => [:change, :secret, :text, *BASIC_S],
    EditBox    => [:change, :text, *BASIC_S],
    Effect     => [:radius, :distance, :inner, *(COLOR_S | BASIC_S)],
    Image      => MOUSE_S | BASIC_S,
    ListBox    => [:change, :items, :choose, *BASIC_S],
    # Pattern    => [:angle, :radius, *BASIC_S],
    Progress   => BASIC_S,
    Shape      => COLOR_S | MOUSE_S | BASIC_S,
    TextBlock  => [:justify, :align, :leading, *(COLOR_S | MOUSE_S | TEXT_S | BASIC_S)],
    Text       => COLOR_S | MOUSE_S | TEXT_S | BASIC_S }
    .each do |klass, styles|
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
    def self.inherited(subc)
      methc = subc.to_s[/(^|::)(\w+)$/, 2]
              .gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2')
              .gsub(/([a-z\d])([A-Z])/, '\1_\2').downcase
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
