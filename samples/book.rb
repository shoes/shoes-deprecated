require 'yaml'

num = 0
incidents = YAML.load_file('samples/book.yaml')
toc = "<span font_desc='Arial 11px'>"

page = proc do
  flow :margin => 10, :margin_left => 200, :margin_top => 20 do
    text "<span font_desc='Arial 46px'>Incident</span>\n<b>No. #{num + 1}: #{incidents[num][0]}</b>"
  end
  flow :width => 190 do
    text toc
  end
  flow :width => -250, :margin_left => 10 do
    text incidents[num][1]
  end
end

incidents.each_with_index do |(title, story), i|
  toc += "(#{i + 1}) <a href='/incidents/#{i}'>#{title}</a> "
  Shoes.mount("/incidents/#{i}") do
    num = i
    instance_eval &page
  end
end
toc += "</span>"

Shoes.app do
  story = "Please select a story."
  instance_eval &page
end
