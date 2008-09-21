#
# lib/shoes/pack.rb
# Packing apps into Windows, OS X and Linux binaries
#
require 'shoes/shy'
require 'binject'
require 'open-uri'

class Shoes
  module Pack
    def self.rewrite a, before, hsh
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(/\#\{(\w+)\}/) { hsh[$1] }
        end
      end
    end

    def self.pkg(platform, opt)
      url = 
        case opt
        when I_YES; "http://hacketyhack.net/pkg/#{platform}/shoes"
        when I_NOV; "http://hacketyhack.net/pkg/#{platform}/shoes-novideo"
        end
      if url
        url = "http://hacketyhack.net" + open(url).read.strip
        save = File.join(LIB_DIR, File.basename(url))
        if File.exists? save; open(save)
        else                  open(url)
        end
      end
    end

    def self.exe(script, opt, &blk)
      size = File.size(script)
      f = File.open(script)
      exe = Binject::EXE.new(File.join(DIR, "static", "stubs", "blank.exe"))
      size += script.length
      exe.inject("SHOES_FILENAME", File.basename(script))
      size += File.size(script)
      exe.inject("SHOES_PAYLOAD", f)
      f2 = pkg("win32", opt)
      if f2
        size += File.size(f2.path)
        f3 = File.open(f2.path, 'rb')
        exe.inject("SHOES_SETUP", f3)
      end

      count, last = 0, 0.0
      exe.save(script.gsub(/\.\w+$/, '') + ".exe") do |len|
        count += len
        prg = count.to_f / size.to_f
        blk[last = prg] if blk and prg - last > 0.02 and prg < 1.0
      end

      f.close
      if f2
        f2.close
        f3.close
      end
      blk[1.0] if blk
    end

    def self.dmg(script, opt, &blk)
      name = File.basename(script).gsub(/\.\w+$/, '')
      app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
      vol_name = name.capitalize.gsub(/[-_](\w)/) { " " + $1.capitalize }
      app_app = "#{app_name}.app"
      vers = [1, 0]

      tmp_dir = File.join(LIB_DIR, "+dmg")
      FileUtils.rm_rf(tmp_dir)
      FileUtils.mkdir_p(tmp_dir)
      FileUtils.cp(File.join(DIR, "static", "stubs", "blank.hfz"),
                   File.join(tmp_dir, "blank.hfz"))
      app_dir = File.join(tmp_dir, app_app)
      res_dir = File.join(tmp_dir, app_app, "Contents", "Resources")
      mac_dir = File.join(tmp_dir, app_app, "Contents", "MacOS")
      [res_dir, mac_dir].map { |x| FileUtils.mkdir_p(x) }
      FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), app_dir)
      FileUtils.cp(File.join(DIR, "static", "Shoes.icns"), res_dir)
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
      FileUtils.cp(File.join(DIR, "static", "stubs", "cocoa-install"),
        File.join(mac_dir, "cocoa-install"))

      dmg = Binject::DMG.new(File.join(tmp_dir, "blank.hfz"), vol_name)
      f2 = pkg("osx", opt)
      if f2
        dmg.grow(10)
        dmg.inject_file("setup.dmg", f2.path)
      end
      dmg.inject_dir(app_app, app_dir)
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/#{name}-launch")
      dmg.chmod_file(0755, "#{app_app}/Contents/MacOS/cocoa-install")
      dmg.save(script.gsub(/\.\w+$/, '') + ".dmg") do |perc|
        blk[perc * 0.01] if blk
      end
      FileUtils.rm_rf(tmp_dir)
      blk[1.0] if blk
    end

    def self.linux(script, opt, &blk)
      name = File.basename(script).gsub(/\.\w+$/, '')
      app_name = name.capitalize.gsub(/[-_](\w)/) { $1.capitalize }
      run_path = script.gsub(/\.\w+$/, '') + ".run"
      tgz_path = script.gsub(/\.\w+$/, '') + ".tgz"
      tmp_dir = File.join(LIB_DIR, "+run")
      FileUtils.mkdir_p(tmp_dir)
      pkgf = pkg("linux", opt)
      prog = 1.0
      if pkgf
        size = Shy.hrun(pkgf)
        pblk = Shy.progress(size) do |name, perc, left|
          blk[perc * 0.5]
        end if blk
        Shy.xzf(pkgf, tmp_dir, &pblk)
        prog -= 0.5
      end

      FileUtils.cp(script, File.join(tmp_dir, File.basename(script)))
      File.open(File.join(tmp_dir, "sh-install"), 'wb') do |a|
        rewrite a, File.join(DIR, "static", "stubs", "sh-install"),
          'SCRIPT' => "./#{File.basename(script)}"
      end
      FileUtils.chmod 0755, File.join(tmp_dir, "sh-install")

      raw = Shy.du(tmp_dir)
      File.open(tgz_path, 'wb') do |f|
        pblk = Shy.progress(raw) do |name, perc, left|
          blk[prog + (perc * prog)]
        end if blk
        Shy.czf(f, tmp_dir, &pblk)
      end
       
      md5, fsize = Shy.md5sum(tgz_path), File.size(tgz_path)
      File.open(run_path, 'wb') do |f|
        rewrite f, File.join(DIR, "static", "stubs", "blank.run"),
          'CRC' => '0000000000', 'MD5' => md5, 'LABEL' => app_name, 'NAME' => name,
          'SIZE' => fsize, 'RAWSIZE' => (raw / 1024) + 1, 'TIME' => Time.now, 'FULLSIZE' => raw
        File.open(tgz_path, 'rb') do |f2|
          f.write f2.read(8192) until f2.eof
        end
      end
      FileUtils.chmod 0755, run_path
      FileUtils.rm_rf(tgz_path)
      FileUtils.rm_rf(tmp_dir)
      blk[1.0] if blk
    end
  end

  I_NET = "No, download Shoes if it's absent."
  I_YES = "Yes, I want Shoes included."
  I_NOV = "Yes, include Shoes, but without video support."
  PackMake = proc do
    background "#EEE"

    @page1 = stack do
      stack do
        background white
        stack :margin => 20 do
          selt = proc { @sel1.toggle; @sel2.toggle }
          @path = ""
          @shy_path = nil
          @sel1 =
            flow do
              para "File:"
              inscription " (or a ", link("directory", &selt), ")"
              edit1 = edit_line :width => -120
              button "Browse...", :width => 100 do
                @path = edit1.text = ask_open_file
                est_recount
              end
            end
          @sel2 =
            flow :hidden => true do
              para "Directory:"
              inscription " (or a ", link("single file", &selt), ")"
              edit2 = edit_line :width => -120
              button "Folder...", :width => 100 do
                @path = edit2.text = ask_open_folder
                est_recount
              end
            end

          para "Package for:"
          flow :margin_left => 20 do
            @shy = check
            para "Shoes (.shy)", :margin_right => 20
            @exe = check
            para "Windows", :margin_right => 20
            @dmg = check
            para "OS X", :margin_right => 20
            @run = check
            para "Linux"
          end

          para "Include Shoes with your app? ", :margin => 0
          @inc = list_box :items => [I_NET, I_YES, I_NOV], :width => 0.6 do
            est_recount
          end
          inscription "(This option doesn't apply to Shoes .shy files.)"
        end
      end

      stack :margin => 20 do
        @est = para "Estimated size of each app: ", strong("0k"), :margin => 0, :margin_bottom => 4
        def est_recount
          base = 
            case @inc.text
            when I_NET; 70
            when I_YES; 7000
            when I_NOV; 2500
            end
          base += ((File.directory?(@path) ? Shy.du(@path) : File.size(@path)) rescue 0) / 1024
          @est.replace "Estimated size of each app: ", strong(base > 1024 ?
            "%0.1fM" % [base / 1024.0] : "#{base}K")
        end
        def build_thread
          @shy_path = nil
          if File.directory? @path
            @shy_path = @path.gsub(%r![\\/]+$!, '') + ".shy"
          elsif @shy.checked?
            @shy_path = @path.gsub(/\.\w+$/, '') + ".shy"
          end
          if @shy_path and not @shy_meta
            @page_shy.show
            @shy_para.text = File.basename(@shy_path)
            @shy_launch.items = Shy.launchable(@path)
            return
          end
          @page2.show 
          @path2.replace File.basename(@path)
          Thread.start do
            begin
              sofar, stage = 0.0, 1.0 / [@shy.checked?, @exe.checked?, @dmg.checked?, @run.checked?].
                select { |x| x }.size
              blk = proc do |frac|
                @prog.style(:width => sofar + (frac * stage))
              end

              if @shy_path
                @status.replace "Compressing the script's folder."
                pblk = Shy.progress(Shy.du(@path)) do |name, perc, left|
                  blk[perc]
                end
                Shy.c(@shy_path, @shy_meta, @path, &pblk)
                @path = @shy_path
                @prog.style(:width => sofar += stage)
              end
              if @exe.checked?
                @status.replace "Working on an .exe for Windows."
                Shoes::Pack.exe(@path, @inc.text, &blk)
                @prog.style(:width => sofar += stage)
              end
              if @dmg.checked?
                @status.replace "Working on a .dmg for Mac OS X."
                Shoes::Pack.dmg(@path, @inc.text, &blk)
                @prog.style(:width => sofar += stage)
              end
              if @run.checked?
                @status.replace "Working on a .run for Linux."
                Shoes::Pack.linux(@path, @inc.text, &blk)
                @prog.style(:width => sofar += stage)
              end
              if @shy_path and not @shy.checked?
                FileUtils.rm_rf(@shy_path)
              end

              @prog.style(:width => 1.0)
              @page2.hide
              @page3.show 
              @path3.replace File.basename(@path)
            rescue => e
              error(e)
            end
          end
        end
        inscription "Using the latest Shoes build (0.r#{Shoes::REVISION})", :margin => 0
        flow :margin_top => 10, :margin_left => 310 do
          button "OK", :margin_right => 4 do
            @page1.hide
            build_thread
          end
          button "Cancel" do
            close
          end
        end
      end
    end

    @page_shy = stack :hidden => true do
      stack do
        background white
        stack :margin => 20 do
          para "Details for:", :margin => 4
          @shy_para = para "", :size => 20, :margin => 4
          flow do
            stack :margin => 10, :width => 0.4 do
              para "Name of app:"
              @shy_name = edit_line :width => 1.0
            end
            stack :margin => 10, :width => 0.4 do
              para "Version:"
              @shy_version = edit_line :width => 120
            end
            stack :margin => 10, :width => 0.4 do
              para "Creator"
              @shy_creator = edit_line :width => 1.0
            end
            stack :margin => 10, :width => 0.5 do
              para "Launch"
              @shy_launch = list_box
            end
          end
        end
      end

      stack :margin => 20 do
        flow :margin_top => 10, :margin_left => 310 do
          button "OK", :margin_right => 4 do
            @shy_meta = Shy.new
            @shy_meta.name = @shy_name.text
            @shy_meta.creator = @shy_creator.text
            @shy_meta.version = @shy_version.text
            @shy_meta.launch =  @shy_launch.text
            @page_shy.hide
            build_thread
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
          @status = para "", :margin => 4
        end
      end

      stack :margin => 20 do
        stack :width => -20, :height => 24 do
          @prog = background "#{DIR}/static/stripe.png", :curve => 7
          background "rgb(0, 0, 0, 100)".."rgb(120, 120, 120, 0)", :curve => 6, :height => 16
          background "rgb(120, 120, 120, 0)".."rgb(0, 0, 0, 100)", :curve => 6, 
            :height => 16, :top => 8
          border "rgb(60, 60, 60, 80)", :curve => 7, :strokewidth => 2
        end
      end
    end

    @page3 = stack :hidden => true do
      stack do
        background white
        stack :margin => 20 do
          para "Completed:", :margin => 4
          @path3 = para "", :size => 20, :margin => 4
          para "Your files are done, you may close this window.", :margin => 4
        end
      end
    end

    start do
      @exe.checked = true
      @dmg.checked = true
      @run.checked = true
      @inc.choose I_NET
    end
  end
end
