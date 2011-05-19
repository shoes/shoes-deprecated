require File.expand_path('make/make')

include FileUtils

class MakeDarwin
  extend Make

  class << self
    def copy_ext xdir, libdir
      Dir.chdir(xdir) do
        `ruby extconf.rb; make`
      end
      copy_files "#{xdir}/*.bundle", libdir
    end

    def copy_deps_to_dist
      if ENV['SHOES_DEPS_PATH']
        dylibs = IO.readlines("make/darwin/dylibs.shoes").map(&:chomp)
        if ENV['VIDEO']
          dylibs += IO.readlines("make/darwin/dylibs.video").map(&:chomp)
        end
        dylibs.each do |libn|
          cp "#{ENV['SHOES_DEPS_PATH']}/#{libn}", "dist/"
        end.each do |libn|
          next unless libn =~ %r!^lib/(.+?\.dylib)$!
          libf = $1
          sh "install_name_tool -id /tmp/dep/#{libn} dist/#{libf}"
          ["dist/#{NAME}-bin", *Dir['dist/*.dylib']].each do |lib2|
            sh "install_name_tool -change /tmp/dep/#{libn} @executable_path/#{libf} #{lib2}"
            sh "install_name_tool -change /tmp/dep/#{libn} @executable_path/#{libf} dist/shoes-bin"
          end
          sh "install_name_tool -change /tmp/dep/lib/libcairo.2.dylib @exectuable_path/libcairo.2.dylib dist/libcairo.2.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libcairo.2.dylib @executable_path/libcairo.2.dylib dist/libcairo.2.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libgif.4.dylib @executable_path/libgif.4.dylib dist/libgif.4.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libglib-2.0.0.dylib @executable_path/libglib-2.0.0.dylib dist/libglib-2.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libgmodule-2.0.0.dylib @executable_path/libgmodule-2.0.0.dylib dist/libgmodule-2.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libgobject-2.0.0.dylib @executable_path/libgobject-2.0.0.dylib dist/libgobject-2.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libgthread-2.0.0.dylib @executable_path/libgthread-2.0.0.dylib dist/libgthread-2.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libiconv.2.dylib @executable_path/libiconv.2.dylib dist/libiconv.2.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libintl.8.dylib @executable_path/libintl.8.dylib dist/libintl.8.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libjpeg.8.dylib @executable_path/libjpeg.8.dylib dist/libjpeg.8.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libpango-1.0.0.dylib @executable_path/libpango-1.0.0.dylib dist/libpango-1.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libpangocairo-1.0.0.dylib @executable_path/libpangocairo-1.0.0.dylib dist/libpangocairo-1.0.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libpixman-1.0.dylib @executable_path/libpixman-1.0.dylib dist/libpixman-1.0.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libpng14.14.dylib @executable_path/libpng14.14.dylib dist/libpng14.14.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libportaudio.2.dylib @executable_path/libportaudio.2.dylib dist/libportaudio.2.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libruby.dylib @executable_path/libruby.dylib dist/libruby.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/gems/1.9.1/gems/hpricot-0.8.1/lib/fast_xs.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/gems/1.9.1/gems/hpricot-0.8.1/lib/hpricot_scan.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/gems/1.9.1/gems/json-shoes-1.1.3/lib/json/ext/generator.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/gems/1.9.1/gems/json-shoes-1.1.3/lib/json/ext/parser.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/gems/1.9.1/gems/sqlite3-ruby-1.3.0/lib/sqlite3_native.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/bloops.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libportaudio.2.dylib @executable_path/libportaudio.2.dylib dist/ruby/lib/x86_64-darwin10.7.0/bloops.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libiconv.2.dylib @executable_path/libiconv.2.dylib dist/ruby/lib/x86_64-darwin10.7.0/iconv.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/etc.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/iconv.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/libshoes.dylib"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/shoes-bin"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/bigdecimal.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/binject.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/bloops.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/chipmunk.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/continuation.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/coverage.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/curses.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/dbm.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest/bubblebabble.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest/md5.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest/rmd160.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest/sha1.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest/sha2.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/digest.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/dl/callback.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/dl.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/big5.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/cp949.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/emacs_mule.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/encdb.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/euc_jp.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/euc_kr.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/euc_tw.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/gb18030.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/gb2312.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/gbk.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_1.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_10.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_11.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_13.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_14.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_15.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_16.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_2.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_3.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_4.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_5.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_6.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_7.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_8.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/iso_8859_9.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/koi8_r.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/koi8_u.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/shift_jis.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/big5.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/chinese.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/emoji.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/emoji_iso2022_kddi.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/emoji_sjis_docomo.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/emoji_sjis_kddi.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/emoji_sjis_softbank.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/escape.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/gb18030.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/gbk.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/iso2022.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/japanese.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/japanese_euc.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/japanese_sjis.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/korean.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/single_byte.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/transdb.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/utf8_mac.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/trans/utf_16_32.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/utf_16be.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/utf_16le.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/utf_32be.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/utf_32le.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/enc/windows_1251.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/etc.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/fcntl.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/fiber.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/fiddle.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/ftsearchrt.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/iconv.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/io/nonblock.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/io/wait.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/json/ext/generator.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/json/ext/parser.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/mathn/complex.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/mathn/rational.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/nkf.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/objspace.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/openssl.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/psych.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/pty.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/racc/cparse.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/readline.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/ripper.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/sdbm.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/socket.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/stringio.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/strscan.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/syck.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/syslog.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/tcltklib.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/tkutil.bundle"
          sh "install_name_tool -change /tmp/dep/lib/libruby.1.9.1.dylib @executable_path/libruby.dylib dist/ruby/lib/x86_64-darwin10.7.0/zlib.bundle"
        end
        if ENV['VIDEO']
          mkdir_p "dist/plugins"
          sh "cp -r deps/lib/vlc/**/*.dylib dist/plugins"
          sh "strip -x dist/*.dylib"
          sh "strip -x dist/plugins/*.dylib"
          sh "strip -x dist/ruby/lib/**/*.bundle"
        end
      end
    end

    def setup_system_resources
      rm_rf "#{APPNAME}.app"
      mkdir "#{APPNAME}.app"
      mkdir "#{APPNAME}.app/Contents"
      cp_r "dist", "#{APPNAME}.app/Contents/MacOS"
      mkdir "#{APPNAME}.app/Contents/Resources"
      mkdir "#{APPNAME}.app/Contents/Resources/English.lproj"
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/App.icns\""
      sh "ditto \"#{APP['icons']['osx']}\" \"#{APPNAME}.app/Contents/Resources/App.icns\""
      rewrite "platform/mac/Info.plist", "#{APPNAME}.app/Contents/Info.plist"
      cp "platform/mac/version.plist", "#{APPNAME}.app/Contents/"
      rewrite "platform/mac/pangorc", "#{APPNAME}.app/Contents/MacOS/pangorc"
      cp "platform/mac/command-manual.rb", "#{APPNAME}.app/Contents/MacOS/"
      rewrite "platform/mac/shoes-launch", "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-launch"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}-bin"
      rewrite "platform/mac/shoes", "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      chmod 0755, "#{APPNAME}.app/Contents/MacOS/#{NAME}"
      # cp InfoPlist.strings YourApp.app/Contents/Resources/English.lproj/
      `echo -n 'APPL????' > "#{APPNAME}.app/Contents/PkgInfo"`
    end

    def make_stub
      ENV['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
      sh "gcc -O -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -framework Cocoa -o stub platform/mac/stub.m -I."
    end

    def make_app(name)
      bin = "#{name}-bin"
      rm_f name
      rm_f bin
      sh "#{CC} -Ldist -o #{bin} bin/main.o #{LINUX_LIBS} -lshoes -arch x86_64"
    end

    def make_so(name)
      ldflags = LINUX_LDFLAGS.sub! /INSTALL_NAME/, "-install_name @executable_path/lib#{SONAME}.#{DLEXT}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
      %w[libpostproc.dylib libavformat.dylib libavcodec.dylib libavutil.dylib libruby.dylib].each do |libn|
        sh "install_name_tool -change /tmp/dep/lib/#{libn} ./deps/lib/#{libn} #{name}"
      end
    end

    def make_installer
      dmg_ds, dmg_jpg = "platform/mac/dmg_ds_store", "static/shoes-dmg.jpg"
      if APP['dmg']
        dmg_ds, dmg_jpg = APP['dmg']['ds_store'], APP['dmg']['background']
      end

      mkdir_p "pkg"
      rm_rf "dmg"
      mkdir_p "dmg"
      cp_r "#{APPNAME}.app", "dmg"
      unless ENV['APP']
        mv "dmg/#{APPNAME}.app/Contents/MacOS/samples", "dmg/samples"
      end
      ln_s "/Applications", "dmg/Applications"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-bin"
      sh "chmod +x dmg/\"#{APPNAME}.app\"/Contents/MacOS/#{NAME}-launch"
      sh "DYLD_LIBRARY_PATH= platform/mac/pkg-dmg --target pkg/#{PKG}.dmg --source dmg --volname '#{APPNAME}' --copy #{dmg_ds}:/.DS_Store --mkdir /.background --copy #{dmg_jpg}:/.background" # --format UDRW"
      rm_rf "dmg"
    end
  end
end
