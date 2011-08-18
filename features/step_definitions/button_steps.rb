Given /^a Shoes application$/ do
  @app = Shoes.app
end

When /^I append a button to the main window$/ do
  @app.append do
    button "hello"
  end
end

Then /^I should see a button$/ do
  @app.elements.find{|e| e.class == Shoes::Button}.should_not be_nil
end

