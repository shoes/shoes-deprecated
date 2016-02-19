module Vlc_path
  require 'yaml'
  require 'fileutils'
 
  def load (yamlp=nil)
    # do we have a vlc.yaml
    vlc_app_path = nil
    vlc_plugin_path = nil
    mostly_ok = false
    if yamlp && (File.exist? yamlp)
      cfg = YAML.load_file(yamlp)
      vlc_app_path = cfg['vlc_app_path']
      vlc_plugin_path = cfg['vlc_plugin_path']
      mostly_ok = true
    end
    # Look in standard locations
    if !vlc_app_path
      case RUBY_PLATFORM
        when /ming/
          if ENV['ProgramFiles(x86)']
            vlc_app_path = "C:/Program Files (x86)/VideoLAN/VLC/libvlc.dll"
            vlc_plugin_path = "C:/Program Files (x86)/VideoLan/VLC/plugins"
          else
            vlc_app_path = "C:/Program Files/VideoLAN/VLC/libvlc.dll"
            vlc_plugin_path = "C:/Program Files/VideoLan/VLC/plugins"
          end
        when /linux/
          Dir.glob('/usr/lib/libvlc.so*') do |p|
            vlc_app_path = p if ! File.symlink?(p)
          end
          vlc_plugin_path = '/usr/lib/vlc/plugins' if vlc_app_path
        when /darwin/
          Dir.glob('/Applications/VLC.app/Contents/MacOS/lib/libvlc.*dylib') do |p|
            vlc_app_path = p if ! File.symlink?(p)
          end
          vlc_plugin_path = "/Applications/VLC.app/Contents/MacOS/plugins" if vlc_app_path
      end

      begin 
        # it's a Windows Ruby thing with the spaces in the path
        Dir.chdir(File.dirname(vlc_app_path)) do |p|
          mostly_ok = File.exist? File.basename(vlc_app_path)
        end
      rescue 
        puts "vlc not at standard location "
        vlc_app_path = nil
        mostly_ok = false
      end
    end
    if vlc_app_path && vlc_plugin_path && mostly_ok
        ENV['VLC_APP_PATH'] = vlc_app_path
        ENV['VLC_PLUGIN_PATH'] = vlc_plugin_path
        #puts "Yaml load: #{vlc_app_path} #{vlc_plugin_path}"
    end
  end
  module_function (:load)

  # save env vars to vlc.yaml
  def save(yamlp)
    tree = { "vlc_app_path" => "#{ENV['VLC_APP_PATH']}",
      "vlc_plugin_path" => "#{ENV['VLC_PLUGIN_PATH']}"
    }
    File.open(yamlp, 'w') {|f| YAML.dump(tree, f) }
  end
  module_function (:save)
  
  # Make sure, as best we can that dlload() will succeed
  # Mostly a windows path problem
  def check (vlc_p, plug_p)
    return false if ! (File.directory? plug_p)
    have = false
    begin 
      Dir.chdir(File.dirname(vlc_p)) do |p|
        if File.exist? File.basename(vlc_p)
          have = true
        end
      end
    rescue
      have = false
    end
    return have
  end
  module_function(:check)
  
end
