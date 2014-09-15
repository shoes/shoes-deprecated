# winject.rb - structured like the 'C' extension
module Winject
  class EXE
    def initialize filepath
      puts "Winject init from #{filepath}"
      return self
    end
    
    def save filepath
      puts "Winject writing #{filepath}"
    end
    
    def inject (name, contents)
      puts "injecting #{name}"
    end
  end
end
