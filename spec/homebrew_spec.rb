# require 'rspec'
require_relative '../make/darwin/homebrew'

describe Homebrew do
  let(:shell) { double }
  let(:brew) { Homebrew.new }

  before :each do
    brew.shell = shell
  end

  it "should install a new package" do
    shell.stub(:brew_installed?) {false}
    shell.should_receive(:run).with "brew install cairo"
    brew.install "cairo"
  end

  it "should default to non-universal" do
    brew.should_not be_universal
  end

  it "should become universal" do
    brew.universal
    brew.should be_universal
  end

  it "should add a custom remote" do
    remote = "shoes"
    remote_url = "https://github.com/wasnotrice/homebrew.git"
    shell.stub(:git_repo_has_remote?) { false }
    shell.should_receive(:git_remote_add).with(remote, remote_url)
    shell.should_receive(:git_fetch).with(remote)
    brew.add_custom_remote
  end

  it "should remove custom remote" do
    shell.stub(:git_repo_has_remote?) { true }
    shell.should_receive(:git_remote_rm).with 'shoes'
    brew.remove_custom_remote
  end

  it "should checkout custom formulas" do
    branch = "shoes"
    brew.custom_formulas.each do |f|
      shell.should_receive(:run).with "git checkout shoes/#{branch} Library/Formula/#{f}.rb"
    end
    brew.add_custom_formulas
  end

  it "should install correct packages" do
    packages = %w(gettext glib pixman cairo pango jpeg giflib portaudio)
    brew.stub(:installed?) { false }
    packages.each do |p|
      shell.should_receive(:run).with /^brew install #{p}/
    end
    shell.should_receive(:run).with "brew link cairo"
    brew.install_packages
  end

  describe ShellCommandRunner do
    before :each do
      @shell = ShellCommandRunner.new
    end

    it "has git command" do
      @shell.git_command.should_not be_nil
    end

    it "has brew command" do
      @shell.brew_command.should_not be_nil
    end
  end
end

