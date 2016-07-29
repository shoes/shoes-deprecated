# check Shoes::VERSION constants. Not pretty
Shoes.app do
  stack do
    clist = []
    Shoes.constants.sort.each do |c|
     if c[/VERSION/] 
       t = "Shoes::#{c}"
       clist << "#{c} = #{eval t}"
     end
    end
    @eb = edit_box  clist.join("\n"), :width => 400, height: 400
    button 'quit' do
      Shoes.quit
    end
  end
end
