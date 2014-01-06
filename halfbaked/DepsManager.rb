
depdir = "deps"  # deps files here

class Arch
  
end


Shoes.app width: 600, height: 800 do
  archpathlist = Dir.glob("#{depdir}/*.yaml")
  arch = {}
  archlist = []
  archpathlist.each do |p| 
    a = File.basename(p, ".yaml")
    arch[a] = p
    archlist << a
  end
  archsel = "None"
  if File.exists? "crosscompile"
    File.open('crosscompile','r') do |f|
      str = f.readline
      archsel = str.split('=')[1].strip
    end
  end
  stack do
    flow do 
      @archb = list_box items: archlist, choose: archsel do
      end
      button "Load" do
      end
      para "New:" 
      @newarch = edit_line "" do 
      end
      button "Create" do
        narch = @newarch.text
        @archb.items = @archb.items << narch
      end
    end
  end
end
