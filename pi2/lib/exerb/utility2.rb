
#==============================================================================#
# $Id: utility2.rb,v 1.2 2005/04/17 15:17:26 yuya Exp $
#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

module Exerb::Utility2

  def self.loaded_features(reject_list = [])
    reject_list << File.expand_path(__FILE__)

    return $".collect { |filename|
      case filename.downcase
      when /\.rb$/o  then type = "script"
      when /\.so$/o  then type = "extension-library"
      when /\.dll$/o then type = "extension-library"
      end
      [type, filename]
    }.collect { |type, filename|
      if File.exist?(filename)
        [type, filename, filename]
      else
        $LOAD_PATH.collect { |dirpath|
          [type, filename, File.join(dirpath, filename)]
        }.find { |type, filename, filepath|
          File.exist?(filepath)
        }
      end
    }.compact.reject { |type, filename, filepath|
      type.nil?
    }.reject { |type, filename, filepath|
      reject_list.index(File.expand_path(filepath))
    }
  end

end # Exerb::Utility2

#==============================================================================#
#==============================================================================#
