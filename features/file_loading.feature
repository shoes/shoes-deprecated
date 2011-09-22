Feature: Simple.rb
  Simple.rb is a very basic application that just consists of some paragraphs,
  links and buttons.

  Scenario: Buttons can be created
    Given a Shoes application in "features/example_programs/simple.rb"
    Then I should see a button with text "Close"
    And I should see a paragraph
    And I should see a link with text "A sample link"

