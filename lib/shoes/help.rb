module Shoes::Manual
  CODE_RE = /\{{3}(?:\s*\#![^\n]+)?(.+?)\}{3}/m
  CODE_STYLE = {:size => 9, :margin => 12}
  INTRO_STYLE = {:size => 12, :weight => "bold", :margin_bottom => 20, :stroke => "#000"}
  SUB_STYLE = {:stroke => "#CCC", :margin_top => 10}
  COLON = ": "

  def dewikify_hi(str, terms, intro = false)
    if terms
      code = []
      str = str.
        gsub(CODE_RE) { |x| code << x; "CODE#[#{code.length-1}]" }.
        gsub(/#{Regexp::quote(terms)}/i, '@\0@').
        gsub(/CODE#\[(\d+)\]/) { code[$1.to_i] }
    end
    dewikify(str, intro)
  end

  def dewikify_p(ele, str, *args)
    str = str.gsub(/\n+\s*/, " ").dump.
      gsub(/`(.+?)`/m, '", code("\1"), "').gsub(/\[\[BR\]\]/i, "\n").
      gsub(/@(.+?)@/m, '", strong("\1", :fill => yellow), "').
      gsub(/'''(.+?)'''/m, '", strong("\1"), "').gsub(/''(.+?)''/m, '", em("\1"), "').
      gsub(/\[\[(\S+?) (.+?)\]\]/m, '", link("\2") { open_link("\1") }, "').
      gsub(/\!(\{[^}\n]+\})?([^!\n]+\.\w+)\!/, '", *args); stack(\1) { image("#{DIR}/static/\2") }; #{ele}("')
    eval("#{ele}(#{str}, *args)")
  end

  def dewikify(str, intro = false)
    proc do
      paras = str.split(/\s*?(\{{3}(?:.+?)\}{3})|\n\n/m).reject { |x| x.empty? }
      if intro
        dewikify_p :para, paras.shift, INTRO_STYLE
      end
      paras.map do |ps|
        if ps =~ CODE_RE
          str = $1.gsub(/\A\n+/, '').chomp
          stack do 
            background rgb(210, 210, 210)
            para code(str), CODE_STYLE 
          end
          stack :margin_bottom => 12, :margin_top => -20 do
            para link("Run this", :stroke => "#777") { run_code(str) },
              :align => "right", :margin => 4
          end
        else
          case ps
          when /\A \* (.+)/m
            dewikify_p :para, $1.split(/^ \* /).join("[[BR]]")
          when /\A==== (.+) ====/
            dewikify_p :caption, $1
          when /\A=== (.+) ===/
            dewikify_p :tagline, $1
          when /\A== (.+) ==/
            dewikify_p :subtitle, $1
          when /\A= (.+) =/
            dewikify_p :title, $1
          else
            dewikify_p :para, ps
          end
        end
      end
    end
  end

  def run_code str
    unless str =~ /Shoes\.app/
      Shoes.app { eval(str) }
    else
      eval(str, TOPLEVEL_BINDING)
    end
  end

  def load_docs str
    return @docs if @docs
    @search = Shoes::Search.new
    @sections, @methods, @mindex = {}, {}, {}
    @docs =
      (str.split(/^= (.+?) =/)[1..-1]/2).map do |k,v|
        sparts = v.split(/^== (.+?) ==/)

        sections = (sparts[1..-1]/2).map do |k2,v2|
          meth = v2.split(/^=== (.+?) ===/)
          k2t = k2[/^(?:The )?([\-\w]+)/, 1]
          @search.add_document :uri => "T #{k2t}", :body => "#{k2}\n#{meth[0]}".downcase

          hsh = {'title' => k2, 'section' => k,
            'description' => meth[0],
            'methods' => (meth[1..-1]/2).map { |_k,_v|
              @search.add_document :uri => "M #{k}#{COLON}#{k2t}#{COLON}#{_k}", :body => "#{_k}\n#{_v}".downcase
              @mindex["#{k2t}.#{_k[/[\w\.]+/]}"] = [k2t, _k]
              [_k, _v]
          }}
          @methods[k2t] = hsh
          [k2t, hsh]
        end

        @search.add_document :uri => "S #{k}", :body => "#{k}\n#{sparts[0]}".downcase
        hsh = {'description' => sparts[0], 'sections' => sections, 
           'class' => "toc" + k.downcase.gsub(/\W+/, '')}
        @sections[k] = hsh
        [k, hsh]
      end
    @search.finish!
    @docs
  end

  def show_search
    @toc.each { |k,v| v.hide }
    @title.replace "Search"
    @doc.clear do
      dewikify_p :para, "Try method names (like `button` or `arrow`) or topics (like `slots`)", :align => 'center'
      flow :margin_left => 60 do
        edit_line :width => -60 do |terms|
          @results.clear do
            termd = terms.text.downcase
            found = termd.empty? ? [] : manual_search(termd)
            para "#{found.length} matches", :align => "center", :margin_bottom => 0
            found.each do |typ, head|
              flow :margin => 4 do
                case typ
                when "S"
                  background "#333", :curve => 4
                  caption strong(link(head, :stroke => white) { open_section(head, terms.text) })
                  para "Section header", Shoes::Manual::SUB_STYLE
                when "T"
                  background "#777", :curve => 4
                  caption strong(link(head, :stroke => "#EEE") { open_methods(head, terms.text) })
                  hsh = @methods[head]
                  para "Sub-section under #{hsh['section']} (#{hsh['methods'].length} methods)", Shoes::Manual::SUB_STYLE
                when "M"
                  background "#CCC", :curve => 4
                  sect, subhead, head = head.split(Shoes::Manual::COLON, 3)
                  para strong(sect, Shoes::Manual::COLON, subhead, Shoes::Manual::COLON, link(head) { open_methods(subhead, terms.text, head) })
                end
              end
            end
          end
        end
      end
      @results = stack
    end
    self.scroll_top = 0
  end

  def open_link(head)
    if @sections.has_key? head
      open_section(head)
    elsif @methods.has_key? head
      open_methods(head)
    elsif @mindex.has_key? head
      head, sub = @mindex[head]
      open_methods(head, nil, sub)
    end
  end

  def open_section(sect_s, terms = nil)
    sect_h = @sections[sect_s]
    sect_cls = sect_h['class']
    @toc.each { |k,v| v.send(k == sect_cls ? :show : :hide) }
    @title.replace sect_s
    @doc.clear(&dewikify_hi(sect_h['description'], terms, true)) 
    self.scroll_top = 0
  end

  def open_methods(meth_s, terms = nil, meth_a = nil)
    meth_h = @methods[meth_s]
    @title.replace meth_h['title']
    @doc.clear do
      unless meth_a
        instance_eval &dewikify_hi(meth_h['description'], terms, true)
      end
      meth_h['methods'].each do |mname, expl|
        if meth_a.nil? or meth_a == mname
          sig, val = mname.split("»", 2)
          stack(:margin_top => 8, :margin_bottom => 8) { 
            background "#333".."#666", :curve => 3, :angle => 90
            tagline sig, (span("»", val, :stroke => "#BBB") if val), :margin => 4 }
          instance_eval &dewikify_hi(expl, terms)
        end
      end
    end
    self.scroll_top = 0
  end

  def manual_search(terms)
    terms += " " if terms.length == 1
    @search.find_all(terms).map do |title, count|
      title.split(" ", 2)
    end
  end
end

def Shoes.make_help_page(str)
  proc do
    extend Shoes::Manual
    docs = load_docs str

    style(Shoes::Code, :stroke => "#C30")
    style(Shoes::LinkHover, :stroke => green, :fill => nil)
    style(Shoes::Para, :size => 9, :stroke => "#332")
    style(Shoes::Tagline, :size => 12, :weight => "bold", :stroke => "#eee", :margin => 6)
    background "#ddd".."#fff", :angle => 90

    stack do
      background black
      stack :margin_left => 118 do
        para "The Shoes Manual", :stroke => "#eee", :margin_top => 8, :margin_left => 17, 
          :margin_bottom => 0
        @title = title docs[0][0], :stroke => white, :margin => 4, :margin_left => 14,
          :margin_top => 0, :weight => "bold"
      end
      background "rgb(66, 66, 66, 180)".."rgb(0, 0, 0, 0)", :height => 0.7
      background "rgb(66, 66, 66, 100)".."rgb(255, 255, 255, 0)", :height => 20, :bottom => 0
    end
    @doc =
      stack :margin_left => 130, :margin_top => 20, :margin_bottom => 50, :margin_right => 20 + gutter,
        &dewikify(docs[0][-1]['description'], true)
    stack :top => 80, :left => 0, :attach => Window do
      @toc = {}
      stack :margin => 12, :width => 130, :margin_top => 20 do
        docs.each do |sect_s, sect_h|
          sect_cls = sect_h['class']
          para strong(link(sect_s, :stroke => black) { open_section(sect_s) }),
            :size => 11, :margin => 4, :margin_top => 0
          @toc[sect_cls] =
            stack :hidden => @toc.empty? ? false : true do
              links = sect_h['sections'].map do |meth_s, meth_h|
                [link(meth_s) { open_methods(meth_s) }, "\n"]
              end.flatten
              links[-1] = {:size => 9, :margin => 4, :margin_left => 10}
              para *links
            end
        end
      end
      stack :margin => 12, :width => 118, :margin_top => 6 do
        background "#330", :curve => 4
        para "Not finding it? Try ", strong(link("Search", :stroke => white) { show_search }), "!", :stroke => "#ddd"
      end
      stack :margin => 12, :width => 118 do
        inscription "Shoes #{Shoes::RELEASE_NAME}\nRevision: #{Shoes::REVISION}",
          :size => 7, :align => "center", :stroke => "#999"
      end
    end
    image :width => 120, :height => 120, :top => -18, :left => 6 do
      image "#{DIR}/static/shoes-icon.png", :width => 100, :height => 100, :top => 10, :left => 10 
      glow 2
    end
  end
rescue => e
  p e.message
  p e.class
end

Shoes::Help = Shoes.make_help_page File.read("#{DIR}/static/manual.txt")
