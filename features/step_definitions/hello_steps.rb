Given /^that things are installed$/ do

end

When /^I assert that true is true$/ do
  @result = true == true
end

Then /^I should get no errors$/ do
  @result.should == true
end

