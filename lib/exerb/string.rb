#
# Cecil's first attempt to add RT_STRING handling for Shoes packaging
#
#==============================================================================#


#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#
class Exerb::String
 
  def initialize (name, value)
    @name = name
    @value = value
  end

  attr_accessor   :name, :value
  
  def pack
    # next two lines work for Ascii
    # fmtstr = "SA#{@value.length}SSSSSSSSSSSSSSS"
    # return [@value.length,@value,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0].pack(fmtstr)
    # We want UTF-16 - who knew?
    intchrs = []
    intchrs[0] = @value.length
    @value.each_byte {|c| intchrs << c.to_i}
    15.times {|i| intchrs << 0}
    return intchrs.pack("S#{intchrs.length}")
  end

end
