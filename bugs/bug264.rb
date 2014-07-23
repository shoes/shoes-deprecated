Shoes.app do
  flow do
    check(checked: true).click do |c|
    # alert "#{c.checked?}"
    para "Check 1 #{c.checked?}\n" 
   end
   para "Check 1"
  end
  flow do
    @c = check(checked: false).click do |c|
      para "Check 2 #{c.checked?}\n"
    end
    para "Check 2"
    @c.checked = true
  end
  para "First line or Last line?\n"
end
