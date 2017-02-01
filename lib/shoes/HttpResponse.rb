# fun - need this in several places.
class Shoes
  class HttpResponse
     # Struct might be better? 
     attr_accessor :headers, :body, :status
     def initalize
       @headers = {}
       @body = ''
       @status = 404
     end
  end
end
