require 'fileutils'
include FileUtils

class Homebrew
  attr_accessor :shell

  def initialize
    @brew_command = ENV['BREW'] || "brew"
    @brew_home = `#{@brew_command} --prefix`.chomp
    @git_command = ENV['GIT'] || "git"
    @remote = "shoes"
    @remote_branch = "shoes"
    @remote_url = "https://github.com/wasnotrice/homebrew.git"
    @shell = ShellCommandRunner.new
  end

  def universal
    @arch = :universal
  end

  def universal?
    @arch == :universal
  end

  def custom_formulas
    ["pixman", "glib", "cairo", "pango", "giflib"]
  end
  
  def install_packages
    install "gettext"
    install "glib"
    install "pixman"
    install "cairo", "--quartz"
    link "cairo"
    install "pango", "--quartz"
    install "jpeg"
    install "giflib"
    install "portaudio"
  end

  def link package
    sh "#{@brew_command} link #{package}"
  end

  def install package, args=""
    if installed?(package)
      puts "#{package} already exists, continuing"
    else
      args << " --universal" if universal?
      sh "#{@brew_command} install #{package} #{args}"
    end
  end

  def installed? package
    `#{@brew_command} list`.split.include?(package)
  end

  def add_custom_formulas
    cd @brew_home do
      custom_formulas.each do |f|
        checkout_formula "#{@remote}/#{@remote_branch}", f
      end
    end
  end

  def remove_custom_formulas
    cd @brew_home do
      custom_formulas.each do |f|
        checkout_formula "master", f
      end
    end
  end

  def checkout_formula branch, formula
    sh "#{@git_command} checkout #{branch} Library/Formula/#{formula}.rb"
  end

  def add_custom_remote remote=@remote, remote_url=@remote_url
    cd @brew_home do
      unless `#{@git_command} remote`.split.include?(remote)
        sh "#{@git_command} remote add #{remote} #{remote_url}"
      end
      sh "#{@git_command} fetch #{remote}"
    end
  end

  def remove_custom_remote
    cd @brew_home do
      if `#{@git_command} remote`.split.include?(@remote)
        sh "#{@git_command} remote rm #{@remote}"
      end
    end
  end

  def sh command
    @shell.run command.strip
  end
end

class ShellCommandRunner
  def run command
    system command
  end
end
