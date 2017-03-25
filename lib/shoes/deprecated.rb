require 'rubygems'

class Shoes 
   module Types
      class Background
         extend Gem::Deprecate

         deprecate :to_pattern, :none, 2017, 10
      end
      
      class Border
         extend Gem::Deprecate

         deprecate :to_pattern, :none, 2017, 10
      end
      
      class Color
         extend Gem::Deprecate

         deprecate :to_pattern, :none, 2017, 10
      end
   end
end
