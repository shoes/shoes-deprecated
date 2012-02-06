# -*- encoding: utf-8 -*-
#
# lib/shoes.rb
# The Shoes base app, both a demonstration and the learning tool for
# using Shoes.
#
require_relative 'shoes/shoes'

#require_relative 'shoes/inspect'
#require_relative 'shoes/cache'
if Object.const_defined? :Shoes
  require_relative 'shoes/image'
end
#require_relative 'shoes/shybuilder'

def Shoes.hook; end

class Encoding
 %w[ASCII_8BIT UTF_16BE UTF_16LE UTF_32BE UTF_32LE US_ASCII].each do |ec|
   eval "#{ec} = '#{ec.sub '_', '-'}'"
 end unless RUBY_PLATFORM =~ /linux/ or RUBY_PLATFORM =~ /darwin/
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
        @shy = true
        require 'shoes/shy'
        base = File.basename(path, ".shy")
        tmpdir = "%s/shoes-%s.%d" % [Dir.tmpdir, base, $$]
        shy = Shy.x(path, tmpdir)
        Dir.chdir(tmpdir)
        Shoes.debug "Loaded SHY: #{shy.name} #{shy.version} by #{shy.creator}"
        path = shy.launch
      else
        @shy = false
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
    if RUBY_VERSION =~ /^1\.9/ and !@shy
      #File.open(path, 'r:utf-8') { |f| f.read }
      IO.read(path).force_encoding("UTF-8")
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
  KEY_S   = [:keydown, :keypress, :keyup]
  COLOR_S = [:stroke, :fill]

  {Background => [:angle, :radius, :curve, *BASIC_S],
   Border     => [:angle, :radius, :curve, :strokewidth, *BASIC_S],
   Canvas     => [:scroll, :start, :finish, *(KEY_S|MOUSE_S|BASIC_S)],
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
    @@types = {}
    def self.inherited subc
      methc = subc.to_s[/(^|::)(\w+)$/, 2].
              gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
              gsub(/([a-z\d])([A-Z])/,'\1_\2').downcase
      @@types[methc] = subc
      Shoes.class_eval %{
        def #{methc}(*a, &b)
          a.unshift Widget.class_variable_get("@@types")[#{methc.dump}]
          widget(*a, &b)
        end
      }
    end
  end
end

def window(*a, &b)
  Shoes.app(*a, &b)
end
