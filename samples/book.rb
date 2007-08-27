class Book < Shoes
  url '/', :index
  url '/incidents/(\d+)', :incident

  def index
    incident(0)
  end

  INCIDENTS = YAML.load_file('samples/book.yaml')

  def table_of_contents
    toc = "<span font_desc='Arial 11px'>"
    INCIDENTS.each_with_index do |(title, story), i|
      toc += "(#{i + 1}) <a href='/incidents/#{i}'>#{title}</a> "
    end
    toc + "</span>"
  end

  def incident(num)
    num = num.to_i
    toc = table_of_contents
    flow :margin => 10, :margin_left => 200, :margin_top => 20 do
      text "<span font_desc='Arial 46px'>Incident</span>\n" +
        "<b>No. #{num + 1}: #{INCIDENTS[num][0]}</b>"
    end
    flow :width => 180, :margin_left => 10 do
      text toc
    end
    flow :width => -250, :margin_left => 10 do
      text INCIDENTS[num][1]
    end
  end
end

Shoes.app :width => 640, :height => 700,
  :title => "Incidents, a Book"
