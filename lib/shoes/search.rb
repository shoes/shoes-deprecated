require("yajl")
require("picky")

Picky.logger = Picky::Loggers::Silent.new

class Search
   Document = Struct.new :id, :uri, :body
   
   Picky.root = LIB_DIR
   
   def initialize
      @documents = []
      @index = Picky::Index.new :terms do
         indexing removes_characters: %r{[^a-z0-9\s\/\-\_\:\"\&\.]}i,
            splits_text_on:     %r{[\s/\-\_\:\"\&/\.]}
         category :uri, :from => lambda { |doc| doc.uri.dup }
         category :body, :from => lambda { |doc| doc.body.dup }
      end
      @search = Picky::Search.new @index do 
         searching removes_characters: %r{[^a-z0-9\s\/\-\_\:\"\&\.]}i,
            splits_text_on:     %r{[\s/\-\_\:\"\&/\.]}
      end
      
      @update = true
      if File.directory?(File.join(Picky.root, "index"))
         file = Dir[File.join(Picky.root, "index", "development", "terms", "*")].first
         if (File.mtime("#{DIR}/static/manual-en.txt") < File.mtime(file))
            @index.load
            @update = false
         end
      end
   end
   
   def add_document(terms = {})
      @documents << Document.new(@documents.size + 1, terms[:uri], terms[:body])
      @index.add @documents[-1] if @update
   end
   
   def find_all(terms)
      retval = []
      results = @search.search(terms)
      results.sort_by { |id| @documents.detect { |n| n.id == id }.uri =~ /#{terms}/ ? 0 : id }
      results.ids.each do |id|
         document = @documents.detect { |n| n.id == id }
         retval << [document.uri] unless document.nil?
      end
      retval
   end
   
   def finish!
      @index.dump if @update
   end
end