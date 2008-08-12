require 'rubygems'
require 'rubygems/dependency_installer'
class << Gem::Ext::ExtConfBuilder
  def build(extension, directory, dest_path, results)
    Kernel.eval(File.read(File.basename(extension)))
    make(dest_path, results) rescue nil
    results
  end
  # alias_method :make__, :make
  # def make(dest_path, results)
  #   raise unless File.exist?('Makefile')
  #   mf = File.read('Makefile')
  #   mf = mf.gsub(/^INSTALL\s*=\s*\$[^$]*/, "INSTALL = '@$(RUBY) -run -e install -- -vp'")
  #   mf = mf.gsub(/^INSTALL_PROG\s*=\s*\$[^$]*/, "INSTALL_PROG = '$(INSTALL) -m 0755'")
  #   mf = mf.gsub(/^INSTALL_DATA\s*=\s*\$[^$]*/, "INSTALL_DATA = '$(INSTALL) -m 0644'")
  #   File.open('Makefile', 'wb') {|f| f.print mf}
  #   make__(dest_path, results)
  # end
end
class << Gem; attr_accessor :loaded_specs end

# STDIN.reopen("/dev/tty") if STDIN.eof?
class NotSupportedByShoes < Exception; end

class Shoes::Setup

  def self.init
    gem_reset
    install_sources if Gem.source_index.find_name('sources').empty?
  end

  def self.gem_reset
    Gem.use_paths(GEM_DIR, [GEM_DIR])
    Gem.source_index.refresh!
  end

  def self.setup_app(setup)
    appt = "Setting up for #{setup.script}"
    Shoes.app :width => 370, :height => 158, :resizable => false, :title => appt do
      background "#EEC".."#AA9"
      image :top => 0, :left => 0 do
        stroke "#FFF"; strokewidth 0.1
        (0..158).step(3) { |i| line 0, i, 370, i }
      end
      @pulse = stack :top => 0, :left => 0
      @logo = image "#{DIR}/static/shoes-icon.png", :top => -20, :right => -20
      stack :margin => 18 do
        title "Shoes Setup", :size => 12, :weight => "bold", :margin => 0
        para "Preparing #{setup.script}", :size => 8, :margin => 0, :margin_top => 8, :width => 220
        progress :width => 1.0, :top => 70, :height => 20
        button "Cancel", :top => 98, :left => 0.4 do
          self.close
        end

        start do
          Thread.start(self.app) do |app|
            begin
              setup.start(app)
            rescue => e
              puts e.message
            end
          end
        end
      end

      animate 10 do |i|
        i %= 10
        @pulse.clear do
          fill black(0.2 - (i * 0.02))
          strokewidth(3.0 - (i * 0.2))
          stroke red(1.0 - (i * 0.1))
          oval(@logo.left - i, @logo.top - i, @logo.width + (i * 2)) 
        end
      end
    end
  end

  attr_accessor :steps, :script

  def initialize(script, &blk)
    @steps = []
    @script = script
    instance_eval &blk
    unless no_steps?
      app = self.class.setup_app(self)
    end
  end

  def no_steps?
    (@steps.map { |s| s[0] }.uniq - [:source]).empty?
  end

  def gem name, version = nil
    arg = "#{name} #{version}".strip
    name, version = arg.split(/\s+/, 2)
    if ['hpricot', 'sqlite3-ruby'].include? name
      warn "#{name} is included with Shoes and cannot be installed as a gem."
    else
      if Gem.source_index.find_name(name, version).empty?
        @steps << [:gem, arg]
      end
    end
  end

  def source uri
    @steps << [:source, uri]
  end

  def start(app)
    old_ui = Gem::DefaultUserInteraction.ui
    ui = Gem::DefaultUserInteraction.ui = Gem::ShoesFace.new(app)
    count, total = 0, @steps.length
    ui.progress count, total

    steps.each do |act, arg|
      case act
      when :gem
        name, version = arg.split(/\s+/, 2)
        count += 1
        ui.say "Looking for #{name}"
        if Gem.source_index.find_name(name, version).empty?
          ui.title "Installing #{name}"
          installer = Gem::DependencyInstaller.new
          installer.install(name, version || Gem::Requirement.default)
          self.class.gem_reset
        end
        gem = Gem.source_index.find_name(name, version).first
        Gem.activate(gem.name, "= #{gem.version}")
        ui.say "Finished installing #{name}"
      when :source
        ui.title "Switching Gem servers"
        ui.say "Pulling from #{arg}"
        Gem.sources.clear << arg
        self.class.gem_reset
      end
      ui.progress count, total
    end
    Gem::DefaultUserInteraction.ui = old_ui

    Shoes.load(@script)
    app.close
  end

  def svn(dir, save_as = nil, &blk)
    dir.gsub! /(.)\/*$/, '\1/'
    if save_as.nil? or save_as.empty?
      save_as = File.join(GEM_DIR, 'svn', '1')
      save_as.succ! while File.exists? save_as
    elsif save_as.index(GEM_DIR) != 0
      save_as = File.join(GEM_DIR, 'svn', save_as)
    end
    mkdir_p(save_as)
    puts "** Pulling down #{dir}..."
    svnuri = URI.parse(dir)
    case svnuri.scheme
    when "http", "https"
      REXML::Document.new(svnuri.open { |f| f.read }).
        each_element("/svn/index/*") do |ele|
          fname, href = ele.attributes['name'], ele.attributes['href']
          case ele.name
          when "file"
            puts "- #{dir}#{href}"
            URI.parse("#{dir}#{href}").open do |f|
              File.open(File.join(save_as, fname), 'wb') do |f2|
                f2 << f.read(16384) until f.eof?
              end
            end
          when "dir"
            svn("#{dir}#{href}", File.join(save_as, fname))
          end
        end
    else
      raise NotSupportedByShoes, "Only HTTP addresses are supported by Shoes's Subversion module."
    end
    if blk
      Dir.chdir(save_as, &blk)
    end
  end

  def self.install_sources
    require 'base64'
    sources_gem = File.join(LIB_DIR, "sources-0.0.1.gem")
    File.open(sources_gem, "wb") do |f|
      f << Base64.decode64( <<-GEM.gsub(/^ +/, '') )
        ZGF0YS50YXIuZ3oAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAADAwMDA2NDQAMDAwMDAwMAAwMDAwMDAwADAwMDAwMDAwMjYw
        ADAwMDAwMDAwMDAwADAxMzMxNQAgMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB1c3RhcgAwMHdoZWVs
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAd2hlZWwAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAwMDAwMDAwADAwMDAwMDAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAfiwgAAKBzRAADyslM0i/OLy1KTi3WK0pioAkw
        AAIzMxMwDQTotIGhCYINFgcKGBsxKBjQxjmooLS4JLEIaH15RmpqDh51hOTR
        PTdEQG5+SmlOqoJ7ai6XgoIDNCUo2CpEK2WUlBRY6eunp+YCU0ZpUmVaflF6
        qh6QUIoFKk1JTVMoTs1J04NqAQoh9AM5qXkpXCA80P4bBaNgFIyCUYAdAAAA
        AP//AwBOIUx0AAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAG1ldGFkYXRhLmd6
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAw
        MDAwNjQ0ADAwMDAwMDAAMDAwMDAwMAAwMDAwMDAwMDYzMAAwMDAwMDAwMDAw
        MAAwMTM0MDAAIDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAdXN0YXIAMDB3aGVlbAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAHdoZWVsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAMDAwMDAwMAAwMDAwMDAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAH4sIAACgc0QAA4SRyW7cMAyG73wKNndPNCkaFDrkmntb9FIEgizR
        thItLiVneftKHk88QYHWMqCF5E/yY9d1+ImX/u069Y9kirynIOX3mYwbnNHF
        pYjQ7COFrJ6Jc32RKA5fD8cj5Eu/3XqEqANJzGlhQxneDX9n+nkyISBeiIvD
        EawuVeJGiNtOfOluPqMQcv2xE7d1g7yEoPlN4o/JZZy1edIj4czp2VnKaNNL
        9EnbcxU4JEamkAphbQZdzEV7v5YOTL8Xx6RmXaYsETr0rgcK2vl6m1KguYrL
        E4oqNFZXTmsbCDWbYTeXtXjQS0mb3E7A0qAXXxS9klmK7n3T6l20jiXWHSad
        FdtkJA7aZzoXZFVLqP4PUMpvp4hAsTSavF9bQ4hdXVd3V/XUzv+cRPs+TENc
        jgfmCq0yCBKbCGQ3RhdH9UR1FmCIizKTdhuLKXHN/+sBYHCe3tleb2QO3EOh
        XNRmbY6Ng0orz+2FXgvrlc+l3w5zd6OY97CPDNqLpZmipWjcOeYPAAAA//8D
        ADLcAkIBAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        AAAAAAAAAAAAAAAA
      GEM
    end
    Gem::Installer.new(sources_gem).install()
  end
end

class Gem::ShoesFace
  class ProgressReporter
    attr_reader :count

    def initialize(prog, status, size, initial_message,
                   terminal_message = "complete")
      @prog = prog
      (@status = status).replace initial_message
      @total = size
      @count = 0.0
    end

    def updated(message)
      @count += 1.0
      @prog.fraction = (@count / @total.to_f) * 0.5
    end

    def done
    end
  end

  def initialize app
    @title, @status, @prog, = app.contents[-1].contents
  end
  def title msg
    @title.replace msg
  end
  def progress count, total
    @prog.fraction = count.to_f / total.to_f
  end
  def ask_yes_no msg
    Kernel.confirm(msg)
  end
  def ask msg
    Kernel.ask(msg)
  end
  def say msg
    @status.replace msg
  end
  def alert msg, quiz=nil
    say(msg)
    ask(quiz) if quiz
  end
  def progress_reporter(*args)
    ProgressReporter.new(@prog, @status, *args)
  end
  def method_missing(*args)
    p args
    nil
  end
end

Shoes::Setup.init
