# winject.rb - just wraps exerb for Shoes purposes
module Winject
  class EXE
    require 'exerb/executable'
    require 'exerb/resource'
    
    # Constants for RT_STRING resources
    SHOES_APP_NAME = 50  # Name of script or shy
    SHOES_DOWNLOAD_SITE = 51  # website download  - no http://
    SHOES_DOWNLOAD_PATH = 52  # /path/to/cgi
    SHOES_VERSION_NEEDED = 52 # version number string - TBD 
    
    # Constants for RC_DATA resources
    SHOES_APP_CONTENT = 128  # contents of script or shy
    SHOES_SYS_SETUP   = 129  # A copy of Shoes installer - the big file
    
    @exe = ''
    attr_accessor :exe
    
    def initialize filepath
      #puts "Winject init from #{filepath}"
      rawpe = ''
      #File.open(filepath, 'r:ASCII-8BIT') {|file| rawpe = file.read}
      File.open(filepath, 'rb') {|file| rawpe = file.read}
      # parse the rawpe into Exerb objects
      return @exe = Exerb::Executable.new(rawpe)
    end
    
    def save filepath
      #puts "Winject writing #{filepath}"
      @exe.write(filepath)
    end
    
    def inject_string (id, contents)
      #puts "injecting string #{id} = #{contents}"
      #@exe.rsrc.each do |rs| 
      #  puts "resource #{rs.id}"
      #end
      @exe.rsrc.add_string( id, contents)
      #puts "=== after inject ===="
      #@exe.rsrc.each do |rs| 
      #  puts "resource #{rs.id}"
      #end
    end
    
    def inject_file (id, io)
      #puts "Winject file #{id}"
      @exe.rsrc.add_rcdata(id, io)
    end
    
    def inject_icons(iconf_path)
      # delete existing RT_ICON's and RT_ICON_GROUP
      @exe.rsrc.remove(Exerb::Win32::Const::RT_ICON)
      @exe.rsrc.remove(Exerb::Win32::Const::RT_GROUP_ICON)
      group_icon = Exerb::Resource::GroupIcon.new

      icos = Exerb::Win32::IconFile.read(iconf_path)
      icos.entries.each_with_index do |entry, index|
        # Exerb::Win32::IconFile::Entry:
        # make a new Icon and copy
        icon = Exerb::Resource::Icon.new
        icon.width     = entry.width
        icon.height    = entry.height
        icon.bit_count = entry.bit_count
        icon.value     = entry.value

        id   = index + 1
        #icon = Exerb::Resource::Icon.read(icos, entry.width, entry.height, entry.bit_count)
        @exe.rsrc.add(Exerb::Win32::Const::RT_ICON, id, icon)
        group_icon.add(id, icon)
      end
      @exe.rsrc.add(Exerb::Win32::Const::RT_GROUP_ICON, 1, group_icon)
    end
    
  end
end
