#
# lib/shoes/pack.rb
# Packing apps into Windows, OS X and Linux binaries
#
require 'shoes/shy'
require 'binject'

class Shoes
  module Pack
    def self.exe(script)
      f = File.open(script)
      exe = Binject::EXE.new(File.join(::DIR, "static", "stubs", "blank.exe"))
      exe.inject("SHOES_FILENAME", File.basename(script))
      exe.inject("SHOES_PAYLOAD", f)
      exe.save(script.gsub(/\.\w+$/, '') + ".exe")
    end

    def self.dmg(script)
      name = File.basename(script).gsub(/\.\w+$/, '')
      app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
      vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
      app_app = "#{app_name}.app"
      vers = [1, 0]

      tmp_dir = File.join(Shoes::LIB_DIR, "+dmg")
      FileUtils.rm_rf(tmp_dir) if File.exists? tmp_dir
      FileUtils.mkdir_p(tmp_dir)
      FileUtils.cp(File.join(::DIR, "static", "stubs", "blank.hfz"),
                   File.join(tmp_dir, "blank.hfz"))
      app_dir = File.join(tmp_dir, app_app)
      res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
      mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
      [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
      FileUtils.cp(File.join(Shoes::DIR, "static", "Shoes.icns"), app_dir)
      FileUtils.cp(File.join(Shoes::DIR, "static", "Shoes.icns"), res_dir)
      File.open(File.join(app_dir, "Contents", "PkgInfo"), 'w') do |f|
        f << "APPL????"
      end
      File.open(File.join(app_dir, "Contents", "Info.plist"), 'w') do |f|
        f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleGetInfoString</key>
  <string>#{app_name} #{vers.join(".")}</string>
  <key>CFBundleExecutable</key>
  <string>#{name}-launch</string>
  <key>CFBundleIdentifier</key>
  <string>org.hackety.#{name}</string>
  <key>CFBundleName</key>
  <string>#{app_name}</string>
  <key>CFBundleIconFile</key>
  <string>Shoes.icns</string>
  <key>CFBundleShortVersionString</key>
  <string>#{vers.join(".")}</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
  <key>IFMajorVersion</key>
  <integer>#{vers[0]}</integer>
  <key>IFMinorVersion</key>
  <integer>#{vers[1]}</integer>
</dict>
</plist>
END
      end
      File.open(File.join(app_dir, "Contents", "version.plist"), 'w') do |f|
        f << <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>BuildVersion</key>
  <string>1</string>
  <key>CFBundleVersion</key>
  <string>#{vers.join(".")}</string>
  <key>ProjectName</key>
  <string>#{app_name}</string>
  <key>SourceVersion</key>
  <string>#{Time.now.strftime("%Y%m%d")}</string>
</dict>
</plist>
END
      end
      File.open(File.join(mac_dir, "#{name}-launch"), 'w') do |f|
        f << <<END
#!/bin/bash
SHOESPATH=/Applications/Shoes.app/Contents/MacOS
APPPATH="${0%/*}"
unset DYLD_LIBRARY_PATH
cd "$APPPATH"
echo "[Pango]" > /tmp/pangorc
echo "ModuleFiles=$SHOESPATH/pango.modules" >> /tmp/pangorc
if [ ! -d /Applications/Shoes.app ]
  then ./cocoa-install
fi
open -a /Applications/Shoes.app "#{File.basename(script)}"
# DYLD_LIBRARY_PATH=$SHOESPATH PANGO_RC_FILE="$APPPATH/pangorc" $SHOESPATH/shoes-bin "#{File.basename(script)}"
END
      end
      FileUtils.cp(script, File.join(mac_dir, File.basename(script)))
      FileUtils.cp(File.join(::DIR, "static", "stubs", "cocoa-install"),
        File.join(mac_dir, "cocoa-install"))

      dmg = Binject::DMG.new(File.join(tmp_dir, "blank.hfz"), vol_name)
      dmg.inject_dir(app_app, app_dir)
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/#{name}-launch")
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/cocoa-install")
      dmg.save(script.gsub(/\.\w+$/, '') + ".dmg")
      # FileUtils.rm_rf(tmp_dir) if File.exists? tmp_dir
    end

    def self.linux(script)
    end
  end

  I_NET = "No, download Shoes if it's absent."
  I_YES = "Yes, I want Shoes included."
  I_VID = "Yes, include Shoes with video support."
  PackMake = proc do
    background "#EEE"

    @page1 = stack do
      stack do
        background white
        stack :margin => 20 do
          para "File:"
          flow do
            @path = edit_line :width => -120
            button "Browse...", :width => 100 do
              @path.text = ask_open_file
              est_recount
            end
          end

          para "Installers:"
          flow :margin_left => 20 do
            @exe = check :margin_top => 4
            para "Windows", :margin_right => 20
            @dmg = check :margin_top => 4
            para "OS X", :margin_right => 20
            @lin = check :margin_top => 4
            para "Linux"
          end

          para "Include Shoes with your app? "
          @inc = list_box :items => [I_NET, I_YES, I_VID], :width => 0.6 do
            est_recount
          end
        end
      end

      stack :margin => 20 do
        @est = para "Estimated size of each app: ", strong("0k"), :margin => 0, :margin_bottom => 4
        def est_recount
          base = 
            case @inc.text
            when I_NET; 70
            when I_YES; 2500
            when I_VID; 7000
            end
          base += File.size(@path.text) / 1024
          @est.replace "Estimated size of each app: ", strong(base > 1024 ?
            "%0.1fM" % [base / 1024.0] : "#{base}K")
        end
        inscription "Using the latest Shoes build (0.r#{Shoes::REVISION})", :margin => 0
        flow :margin_top => 10, :margin_left => 310 do
          button "OK", :margin_right => 4 do
            @page1.hide
            @prog.fraction = 0
            @page2.show 
            @path2.replace File.basename(@path.text)
            Shoes::Pack.exe(@path.text)
            @prog.fraction = 0.3
            Shoes::Pack.dmg(@path.text)
            @prog.fraction = 0.6
            Shoes::Pack.linux(@path.text)
            @prog.fraction = 1.0
          end
          button "Cancel" do
            close
          end
        end
      end
    end

    @page2 = stack :hidden => true do
      stack do
        background white
        stack :margin => 20 do
          para "Packaging:", :margin => 4
          @path2 = para "", :size => 20, :margin => 4
        end
      end

      stack :margin => 20 do
        @prog = progress :width => -20
      end
    end

    start do
      @exe.checked = true
      @dmg.checked = true
      @lin.checked = true
      @inc.choose I_NET
    end
  end
end
