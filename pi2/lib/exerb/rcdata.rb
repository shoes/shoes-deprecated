#
# Cecil's first attempt to add RT_RCDATA handling for Shoes packaging
#
#==============================================================================#


#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#
class Exerb::Rcdata
 
  def initialize (data)
    @data = data
  end

  attr_accessor :data
  
  def pack
    return Exerb::Utility.alignment16(@data)
  end

end
