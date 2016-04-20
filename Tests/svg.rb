# encoding: UTF-8

class SvgTestBase < Test::Unit::TestCase
    class << self
        def init(app)
            @@app = app
        end
        
        def startup; end
        def shutdown; end
    end

    def setup; end
    def teardown; end
end

class SvgTest0 < SvgTestBase

    description "xml, no dimensions"
    def test_svg_basic
        
        assert_not_nil @@app.svg_obj
        assert_instance_of Shoes::Svg, @@app.svg_obj
        
        assert_equal 190, @@app.svg_obj.width
        assert_equal 110, @@app.svg_obj.height
        
        assert_not_nil @@app.svg_obj.handle
        assert_instance_of Shoes::SvgHandle, @@app.svg_obj.handle
        assert_true @@app.svg_obj.group? "#xml_rectangle"
        
        # this svg is smaller than app dimensions => preferred_width == width
        assert_equal 190, @@app.svg_obj.preferred_width
        assert_equal 110, @@app.svg_obj.preferred_height
        assert_equal 0, @@app.svg_obj.offset_x
        assert_equal 0, @@app.svg_obj.offset_y
        assert_equal 90, @@app.svg_obj.dpi
    end
end

class SvgTest1 < SvgTestBase

    description "file, parent dimensions"
    def test_svg_basic
        
        assert_not_nil @@app.svg_obj
        assert_instance_of Shoes::Svg, @@app.svg_obj
        
        # resized to fit at maximum app dimensions
        assert_equal 600, @@app.svg_obj.width
        assert_equal 360, @@app.svg_obj.height
        
        assert_not_nil @@app.svg_obj.handle
        assert_instance_of Shoes::SvgHandle, @@app.svg_obj.handle
        assert_true @@app.svg_obj.group? "#test_rectangle"
        
        # this svg is bigger than app dimensions => preferred_width > width
        assert_equal 1000, @@app.svg_obj.preferred_width
        assert_equal 600, @@app.svg_obj.preferred_height
        assert_equal 0, @@app.svg_obj.offset_x
        assert_equal 0, @@app.svg_obj.offset_y
        assert_equal 90, @@app.svg_obj.dpi
    end
end

class SvgTest2 < SvgTestBase

    description "parent canvas dimensions"
    def test_svg_basic
        
        assert_not_nil @@app.svg_obj
        assert_instance_of Shoes::Svg, @@app.svg_obj
        
        assert_equal 30, @@app.svg_obj.width
        assert_equal 17, @@app.svg_obj.height
    end
end

class SvgTest3 < SvgTestBase

    description "svg object dimensions"
    def test_svg_basic
        
        assert_not_nil @@app.svg_obj
        assert_instance_of Shoes::Svg, @@app.svg_obj
        
        assert_equal 30, @@app.svg_obj.width
        assert_equal 17, @@app.svg_obj.height
    end
end

class SvgTest4 < SvgTestBase

    description "parent and svg object dimensions"
    def test_svg_basic
        
        assert_not_nil @@app.svg_obj
        assert_instance_of Shoes::Svg, @@app.svg_obj
        
        assert_equal 25, @@app.svg_obj.width
        assert_equal 14, @@app.svg_obj.height
    end
end

class SvgTest5 < SvgTestBase

    description "aspect ratio false, Landscape"
    def test_basic_aspectRatioFalse
        assert_equal 48, @@app.svg_obj.width
        assert_equal 32, @@app.svg_obj.height
    end
end

class SvgTest5B < SvgTestBase

    description "aspect ratio 0.5, Landscape"
    def test_basic_aspectRatio_05
        assert_equal 16, @@app.svg_obj.width
        assert_equal 32, @@app.svg_obj.height
    end
end

class SvgTest5C < SvgTestBase

    description "aspect ratio 1.5, Landscape"
    def test_basic_aspectRatio_15
        assert_equal 48, @@app.svg_obj.width
        assert_equal 32, @@app.svg_obj.height
    end
end

class SvgTest6 < SvgTestBase

    description "aspect ratio 0.5, Square"
    def test_basic_aspectRatioCustomLess
        assert_equal 24, @@app.svg_obj.width
        assert_equal 48, @@app.svg_obj.height
    end
end

class SvgTest7 < SvgTestBase

    description "aspect ratio 1.5, Square"
    def test_basic_aspectRatioCustomMore
        assert_equal 48, @@app.svg_obj.width
        assert_equal 32, @@app.svg_obj.height
    end
end

class SvgTest8 < SvgTestBase

    description "xml, group"
    def test_xmlGroup
        assert_equal 180, @@app.svg_obj.preferred_width
        assert_equal 100, @@app.svg_obj.preferred_height
        assert_equal 5, @@app.svg_obj.offset_x
        assert_equal 5, @@app.svg_obj.offset_y
    end
end

class SvgTest9 < SvgTestBase

    description "file, group"
    def test_fileGroup
        assert_equal 980, @@app.svg_obj.preferred_width
        assert_equal 550, @@app.svg_obj.preferred_height
        assert_equal 5, @@app.svg_obj.offset_x
        assert_equal 45, @@app.svg_obj.offset_y
    end
end

class SvgTest10 < SvgTestBase

    description "aspect ratio true, Portrait"
    def test_aspectRatioTrue
        assert_equal 32, @@app.svg_obj.width
        assert_equal 18, @@app.svg_obj.height
    end
end

class SvgTest11 < SvgTestBase

    description "aspect ratio false, Portrait"
    def test_aspectRatioFalse
        assert_equal 32, @@app.svg_obj.width
        assert_equal 48, @@app.svg_obj.height
    end
end

class SvgTest12 < SvgTestBase

    description "aspect ratio 0.5, Portrait"
    def test_aspectRatio_05
        assert_equal 24, @@app.svg_obj.width
        assert_equal 48, @@app.svg_obj.height
    end
end

class SvgTest13 < SvgTestBase

    description "aspect ratio 1.5, Portrait"
    def test_aspectRatio_15
        assert_equal 32, @@app.svg_obj.width
        assert_equal 21, @@app.svg_obj.height
    end
end


