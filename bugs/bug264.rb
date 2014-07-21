Shoes.app do
  flow do
        check(checked: true).click do |c|
          # alert "#{c.checked?}"
          para "#{c.checked?}" 
        end
        para "Check 1"
  end
  para "test"
end
