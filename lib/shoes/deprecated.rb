require 'rubygems'

__END__
# The following code is a sample on how to deprecate Shoes API

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
   end
end
