## old manual -> html processor.
## needs lots of fixing if it's ever used again

   def self.manual_p(str, path)
    str.gsub(/\n+\s*/, ' ')
      .gsub(/&/, '&amp;').gsub(/>/, '&gt;').gsub(/>/, '&lt;').gsub(/"/, '&quot;')
      .gsub(/`(.+?)`/m, '<code>\1</code>').gsub(/\[\[BR\]\]/i, "<br />\n")
      .gsub(/\^(.+?)\^/m, '\1')
      .gsub(/'''(.+?)'''/m, '<strong>\1</strong>').gsub(/''(.+?)''/m, '<em>\1</em>')
      .gsub(/\[\[(http:\/\/\S+?)\]\]/m, '<a href="\1" target="_new">\1</a>')
      .gsub(/\[\[(http:\/\/\S+?) (.+?)\]\]/m, '<a href="\1" target="_new">\2</a>')
      .gsub(/\[\[(\S+?)\]\]/m) do
        ms, mn = Regexp.last_match(1).split('.', 2)
        if mn
          '<a href="' + ms + '.html#' + mn + '">' + mn + '</a>'
        else
          '<a href="' + ms + '.html">' + ms + '</a>'
        end
      end
      .gsub(/\[\[(\S+?) (.+?)\]\]/m, '<a href="\1.html">\2</a>')
      .gsub(/\!(\{[^}\n]+\})?([^!\n]+\.\w+)\!/) do
        x = "static/#{Regexp.last_match(2)}"
        FileUtils.cp("#{DIR}/#{x}", "#{path}/#{x}") if File.exist? "#{DIR}/#{x}"
        '<img src="' + x + '" />'
      end
  end

 
  TITLES = { title: :h1, subtitle: :h2, tagline: :h3, caption: :h4 }

  def self.manual_as(format, *args)
    require 'shoes/search'
    require 'shoes/help'

    case format
    when :shoes
      Shoes.app(width: 720, height: 640, &Shoes::Help)
    else
      extend Shoes::Manual
      man = self
      dir, = args
      FileUtils.mkdir_p File.join(dir, 'static')
      FileUtils.cp "#{DIR}/static/shoes-icon.png", "#{dir}/static"
      %w(manual.css code_highlighter.js code_highlighter_ruby.js)
        .each { |x| FileUtils.cp "#{DIR}/static/#{x}", "#{dir}/static" }
      html_bits = proc do
        proc do |sym, text|
          case sym when :intro
                     div.intro { p { self << man.manual_p(text, dir) } }
          when :code
            pre { code.rb text.gsub(/^\s*?\n/, '') }
          when :colors
            color_names = (Shoes::COLORS.keys * "\n").split("\n").sort
            color_names.each do |color|
              c = Shoes::COLORS[color.intern]
              f = c.dark? ? 'white' : 'black'
              div.color(style: "background: #{c}; color: #{f}") { h3 color; p c }
            end
          when :index
            tree = man.class_tree
            shown = []
            i = 0
            index_p = proc do |k, subs|
              unless shown.include? k
                i += 1
                p "▸ #{k}", style: "margin-left: #{20 * i}px"
                subs.uniq.sort.each do |s|
                  index_p[s, tree[s]]
                end if subs
                i -= 1
                shown << k
              end
            end
            tree.sort.each &index_p
          #   index_page
          when :list
            ul { text.each { |x| li { self << man.manual_p(x, dir) } } }
          when :samples
            folder = File.join DIR, 'samples'
            h = {}
            Dir.glob(File.join folder, '*').each do |file|
              if File.extname(file) == '.rb'
                key = File.basename(file).split('-')[0]
                h[key] ? h[key].push(file) : h[key] = [file]
              end
            end
            h.each do |k, v|
              p "<h4>#{k}</h4>"
              samples = []
              v.each do |file|
                sample = File.basename(file).split('-')[1..-1].join('-')[0..-4]
                samples << "<a href=\"http://github.com/shoes/shoes/raw/master/manual-snapshots/#{k}-#{sample}.png\">#{sample}</a>"
              end
              p samples.join ' '
            end
          else
            send(TITLES[sym] || :p) { self << man.manual_p(text, dir) }
          end
        end
      end

      docs = load_docs(Shoes::Manual.path)
      sections = docs.map { |x,| x }

      docn = 1
      docs.each do |title1, opt1|
        subsect = opt1['sections'].map { |x,| x }
        menu = sections.map do |x|
          [x, (subsect if x == title1)]
        end

        path1 = File.join(dir, title1.gsub(/\W/, ''))
        make_html("#{path1}.html", title1, menu) do
          h2 'The Shoes Manual'
          h1 title1
          man.wiki_tokens opt1['description'], true, &instance_eval(&html_bits)
          p.next do
            text 'Next: '
            a opt1['sections'].first[1]['title'], href: "#{opt1['sections'].first[0]}.html"
          end
        end

        optn = 1
        opt1['sections'].each do |title2, opt2|
          path2 = File.join(dir, title2)
          make_html("#{path2}.html", opt2['title'], menu) do
            h2 'The Shoes Manual'
            h1 opt2['title']
            man.wiki_tokens opt2['description'], true, &instance_eval(&html_bits)
            opt2['methods'].each do |title3, desc3|
              sig, val = title3.split(/\s+»\s+/, 2)
              aname = sig[/^[^(=]+=?/].gsub(/\s/, '').downcase
              a name: aname
              div.method do
                a sig, href: "##{aname}"
                text " » #{val}" if val
              end
              div.sample do
                man.wiki_tokens desc3, &instance_eval(&html_bits)
              end
            end
            if opt1['sections'][optn]
              p.next do
                text 'Next: '
                a opt1['sections'][optn][1]['title'], href: "#{opt1['sections'][optn][0]}.html"
              end
            elsif docs[docn]
              p.next do
                text 'Next: '
                a docs[docn][0], href: "#{docs[docn][0].gsub(/\W/, '')}.html"
              end
            end
            optn += 1
          end
        end

        docn += 1
      end
    end
  end
