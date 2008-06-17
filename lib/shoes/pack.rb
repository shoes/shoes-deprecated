#
# lib/shoes/pack.rb
# Packing apps into Windows, OS X and Linux binaries
#
require 'shoes/shy'
require 'binject'

class Shoes
  def pack_exe(script)
    f = File.open(script)
    exe = Binject::EXE.new(File.join(Shoes::DIR, "static", "stubs", "blank.exe"))
    exe.inject("SHOES_FILENAME", File.basename(script))
    exe.inject("SHOES_PAYLOAD", f)
    exe.save(script.gsub(/\.\w+$/, '') + ".exe")
  end

  def pack_dmg(script)
    tmp_dir = File.join(Shoes::LIB_DIR, "+dmg")
    FileUtils.rm_rf(tmp_dir) if File.exists? tmp_dir
    FileUtils.mkdir_p(tmp_dir)
    dmg = Binject::DMG.new(File.join(Shoes::DIR, "static", "stubs", "blank.hfz"))
    dmg.inject_dir("#{script.capitalize}.app", tmp_dir)
    dmg.save(script.gsub(/\.\w+$/, '') + ".dmg")
  end

  def pack_linux(script)
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
            pack_exe(@path.txt)
            @prog.fraction = 0.3
            pack_dmg(@path.txt)
            @prog.fraction = 0.6
            pack_linux(@path.txt)
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
