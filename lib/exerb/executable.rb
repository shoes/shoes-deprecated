
#==============================================================================#
# $Id: executable.rb,v 1.21 2005/05/03 13:47:01 yuya Exp $
#==============================================================================#

require 'exerb/utility'
require 'exerb/archive'
require 'exerb/error'
require 'exerb/resource'
require 'exerb/win32/pe_file'

#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

class Exerb::Executable

  def initialize(core)
    @core = core
    @rsrc = Exerb::Resource.new_from_pe_binary(@core)
  end

  attr_reader   :core
  attr_accessor :rsrc

  def self.read(input)
    return self.new(Exerb::Utility.read(input))
  end

  def pack
    pe  = Exerb::Win32::PeFile.new_from_binary(@core)
    fh  = pe.nt_headers.file_header
    oh  = pe.nt_headers.optional_header
    rsh = pe.sections.last
    raise(Exerb::ExerbError, "the last section must be resource section") unless rsh.name == '.rsrc'

    packed_rsrc  = @rsrc.pack(rsh.virtual_address)
    aligned_rsrc = Exerb::Utility.alignment(packed_rsrc, oh.file_alignment)
    old_resource_size = Exerb::Utility.align_value(rsh.virtual_size, oh.section_alignment)
    new_resource_size = Exerb::Utility.align_value(packed_rsrc.size, oh.section_alignment)

    @core[rsh.pointer_to_raw_data, rsh.size_of_raw_data] = aligned_rsrc

    fh.time_date_stamp                 = Time.now.to_i
    oh.size_of_initialized_data        = oh.size_of_initialized_data - rsh.size_of_raw_data + aligned_rsrc.size
    oh.size_of_image                   = oh.size_of_image            - old_resource_size + new_resource_size
    oh.resource_directory_virtual_size = packed_rsrc.size
    rsh.virtual_size                   = packed_rsrc.size
    rsh.size_of_raw_data               = aligned_rsrc.size

    fh.update(@core)
    oh.update(@core)
    rsh.update(@core)

    return Exerb::Utility.alignment16(@core)
  end

  def write(output)
    Exerb::Utility.write(output, self.pack, 0755)
  end

end # Exerb::Executable

#==============================================================================#
#==============================================================================#
