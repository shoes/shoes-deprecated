
#==============================================================================#
# $Id: base.rb,v 1.4 2005/04/17 15:56:25 yuya Exp $
#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::Base

  def initialize
    @position = nil
  end

  attr_reader :position

  def self.read(io)
    return self.new.read(io)
  end

  def size
    return self.pack.size
  end

  def read(io)
    @position = io.pos
    return self.unpack(io.read(self.size))
  end

  def update(str)
    str[@position, self.size] = self.pack
  end

end # Exerb::Win32::Struct::Base

#==============================================================================#
#==============================================================================#
