# encoding: UTF-8

require 'test/unit'
require 'test/unit/ui/console/testrunner'

class ColorOpaque < Test::Unit::TestCase
    def setup
        @color = rgb(255,254,253)
    end
    
    def test_color
        assert_not_nil @color
        assert_instance_of Shoes::Color, @color
        
        assert_respond_to(@color, :opaque?)
        assert_true @color.opaque?
        assert_equal @color.red, 255
        assert_equal @color.green, 254
        assert_equal @color.blue, 253
        assert_equal @color.alpha, 255
    end
    
    def test_blackandwhite
        @color = rgb(0,0,0)
        assert_true @color.black?
        
        @color = rgb(255,255,255)
        assert_true @color.white?
    end
    
    def test_lightanddark
        @color = rgb(84,84,84)
        assert_true @color.dark?
        
        @color = rgb(171,171,171)
        assert_true @color.light?
    end
    
    def test_equality
        color = rgb(255,0,0)
        
        # is this desirable ?
        assert_equal 0, rgb(255,0,0) <=> color
        assert_equal 0, rgb(0,0,255) <=> color
        
        assert_true rgb(255,0,0) == color
        assert_true rgb(255,0,0).eql? color
        assert_equal rgb(255,0,0), color
        
        assert_false rgb(0,0,255) == color
        assert_false rgb(0,0,255).eql? color
        assert_not_equal rgb(0,0,255), color
    end
    
    def test_invert
        assert_equal rgb(0,1,2), @color.invert
    end
    
    def test_gray
        @color = gray(89)
        assert_equal rgb(89,89,89), @color
    end
end

class ColorAlpha < Test::Unit::TestCase
    def self.init(app)
        @@app = app
    end
    
    def setup
        @color = rgb(255,254,253,128)
    end
    
    def test_color_alpha
        assert_not_nil @color
        assert_instance_of Shoes::Color, @color
        assert_true @color.alpha < 255
    end
    
    def test_color_transparent
         @color = rgb(255,254,253,0)
         
         assert_respond_to(@color, :transparent?)
         assert_true @color.transparent?
    end
    
    def test_lightanddark
        @color = rgb(84,84,84,128)
        assert_true @color.dark?
        
        @color = rgb(171,171,171,128)
        assert_true @color.light?
    end
    
    def test_blackandwhite
        @color = rgb(0,0,0,128)
        assert_true @color.black?
        
        @color = rgb(255,255,255,128)
        assert_true @color.white?
    end
    
    def test_invert
        assert_equal rgb(0,1,2,128), @color.invert
        assert_true rgb(0,1,2,128) == @color.invert
        assert_equal rgb(0,1,2,128), @color.invert
        
        color = rgb(157,255,25,57)
        assert_equal rgb(98,0,230,57), color.invert
    end
    
    def test_gray
        @color = gray(89, 128)
        assert_equal rgb(89,89,89,128), @color
    end
    
    def test_named
        assert_true rgb(255,0,0,255) == @@app.red
        
        color = @@app.red(0.5)
        assert_equal rgb(255,0,0,128), color
        assert_not_equal rgb(255,0,0,127), color
        assert_not_equal rgb(255,0,0,129), color
    end
end



Shoes.app title: "Testing Shoes::Color" do
    
    ColorAlpha.init self
    
    tests = Test::Unit::TestSuite.new("All Color methods Tests")
    [ColorOpaque, ColorAlpha].each { |t| tests << t.suite }
    
    Shoes.terminal
    @test_result = Test::Unit::UI::Console::TestRunner.run(tests, {use_color: true})
end
