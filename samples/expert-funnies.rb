require 'hpricot'

class Comic
  attr_reader :rss, :title

  def initialize(body)
    @rss = Hpricot.XML(body)
    @title = @rss.at("//channel/title").inner_text
  end

  def items
    @rss.search("//channel/item")
  end

  def latest_image
    @rss.search("//channel/item/description").first.inner_html.scan(/src="([^"]+)/).first
  end
end

Shoes.app :width => 800, :height => 600 do
  @title = "Web Funnies"
  @feeds = [
    "http://xkcd.com/rss.xml",
    "http://feedproxy.google.com/DilbertDailyStrip?format=xml",
    "http://www.smbc-comics.com/rss.php",
    "http://www.daybydaycartoon.com/index.xml",
    "http://www.questionablecontent.net/QCRSS.xml",
    "http://indexed.blogspot.com/feeds/posts/default?alt=rss"
    ]

  stack :width => "100%" do
    title strong @title
  end

  @feeds.each do |feed|
    download feed do |dl|
      stack :width => "100%", :margin => 10, :border => 1 do
        c = Comic.new dl.response.body
        caption strong c.title.upcase
        image c.latest_image.to_s
      end
    end
  end
end
