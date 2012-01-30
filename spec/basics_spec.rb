require "shoes"

describe "A basic Shoes app" do

  it "can be created" do
    @app = Shoes.app do
      para "Hello world!"
    end
    @app.should be_instance_of(Shoes::App)
  end

end
