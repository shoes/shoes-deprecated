# require 'rspec'
require_relative '../make/darwin/homebrew'

describe Homebrew do
  let(:shell) { double }
  let(:brew) { Homebrew.new }

  before :each do
    brew.shell = shell
  end

  it "should install a new package" do
    brew.stub(:installed?) {false}
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
    shell.should_receive(:run).with "git fetch shoes"
    brew.add_custom_remote
  end

  it "should remove custom remote" do
    shell.should_receive(:run).with "git remote rm shoes"
    brew.remove_custom_remote
  end

  it "should checkout custom formulas" do
    branch = "quartz"
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
end

