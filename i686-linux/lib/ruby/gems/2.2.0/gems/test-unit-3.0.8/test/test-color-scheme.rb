class TestUnitColorScheme < Test::Unit::TestCase
  def test_register
    inverted_scheme_spec = {
      "success" => {:name => "red"},
      "failure" => {:name => "green"},
    }
    Test::Unit::ColorScheme["inverted"] = inverted_scheme_spec
    assert_equal({
                   "success" => color("red"),
                   "failure" => color("green"),
                 },
                 Test::Unit::ColorScheme["inverted"].to_hash)
  end

  def test_new_with_colors
    scheme = Test::Unit::ColorScheme.new(:success => color("blue"),
                                         "failure" => color("green",
                                                            :underline => true))
    assert_equal({
                   "success" => color("blue"),
                   "failure" => color("green", :underline => true),
                 },
                 scheme.to_hash)
  end

  def test_new_with_spec
    scheme = Test::Unit::ColorScheme.new(:success => {
                                           :name => "blue",
                                           :bold => true
                                         },
                                         "failure" => {:name => "green"})
    assert_equal({
                   "success" => color("blue", :bold => true),
                   "failure" => color("green"),
                 },
                 scheme.to_hash)
  end

  private
  def color(name, options={})
    Test::Unit::Color.new(name, options)
  end

  class TestFor8Colors < self
    def setup
      @original_term, ENV["TERM"] = ENV["TERM"], nil
      @original_color_term, ENV["COLORTERM"] = ENV["COLORTERM"], nil
      ENV["TERM"] = "xterm"
    end

    def teardown
      ENV["TERM"] = @original_term
      ENV["COLORTERM"] = @original_color_term
    end

    def test_default
      expected_schema = {
        "pass"                => color("green",   :background => true) +
                                 color("white",   :bold       => true),
        "failure"             => color("red",     :background => true) +
                                 color("white",   :bold       => true),
        "pending"             => color("magenta", :background => true) +
                                 color("white",   :bold       => true),
        "omission"            => color("blue",    :background => true) +
                                 color("white",   :bold       => true),
        "notification"        => color("cyan",    :background => true) +
                                 color("white",   :bold       => true),
        "error"               => color("black",   :background => true) +
                                 color("yellow",  :bold       => true),
        "case"                => color("blue",    :background => true) +
                                 color("white",   :bold       => true),
        "suite"               => color("green",   :background => true) +
                                 color("white",   :bold       => true),
        "diff-inserted-tag"   => color("red",     :background => true) +
                                 color("black",   :bold       => true),
        "diff-deleted-tag"    => color("green",   :background => true) +
                                 color("black",   :bold       => true),
        "diff-difference-tag" => color("cyan",    :background => true) +
                                 color("white",   :bold       => true),
        "diff-inserted"       => color("red",     :background => true) +
                                 color("white",   :bold       => true),
        "diff-deleted"        => color("green",   :background => true) +
                                 color("white",   :bold       => true),
      }
      assert_equal(expected_schema,
                   Test::Unit::ColorScheme.default.to_hash)
    end
  end
end
