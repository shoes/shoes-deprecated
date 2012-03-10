require 'shoes/shy'

describe 'Shy' do
  let(:shy_meta_yaml) { "--- !hackety.org,2007/shy \ncreator: Fearless Aeronaut\nlaunch: argon_flyer.rb\nname: Argon Flyer\nversion: 0.1.2\n" }
  let(:shy_meta) {
    s = Shy.new
    s.name = "Argon Flyer"
    s.creator = "Fearless Aeronaut"
    s.version = "0.1.2"
    s.launch = "argon_flyer.rb"
    s
  }

  it "uses the Syck YAML library" do
    YAML.should eq(Syck)
  end

  it "serializes metadata in Shoes format (Syck)" do
    shy_meta.to_yaml.should eq(shy_meta_yaml)
  end

  it "roundtrips .shy metadata" do
    new_shy_meta = YAML.load(shy_meta.to_yaml)
    new_shy_meta.should be_instance_of(Shy)
    new_shy_meta.name.should eq(shy_meta.name)
    new_shy_meta.creator.should eq(shy_meta.creator)
    new_shy_meta.version.should eq(shy_meta.version)
    new_shy_meta.launch.should eq(shy_meta.launch)
  end

  it "extracts metadata from a .shy file into a Shy object" do
    shy = Shy.meta(File.join(File.dirname(__FILE__), "fixtures", "argon_flyer.shy"))
    shy.should be_instance_of(Shy)
  end
end
