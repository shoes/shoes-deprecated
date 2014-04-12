# Borrowed/backported from Shoes 4. Then I modified to fix some bugs
# https://github.com/shoes/shoes4/commit/b0e7cfafe9705f223bcbbd1031acfac02e9f79c6
# needs headers, body, status @vars handling
class Shoes
  class Download
    attr_reader :progress, :response, :content_length, :gui, :transferred, :length 
    #length is preserved for Shoes3 compatibility
    attr_reader :headers, :body, :status, :percent
    UPDATE_STEPS = 100
    
    def initialize(url, opts = {}, &blk)
      @opts = opts
      @blk = blk
      #@gui = Shoes.configuration.backend_for(self)
      @finished = false
      @transferred = 0
      @length = 0.0
      start_download url
    end

    
    def start_download(url)
      require 'open-uri'
      @thread = Thread.new do
        uri_opts = {}
        uri_opts[:content_length_proc] = content_length_proc
        uri_opts[:progress_proc] = progress_proc if @opts[:progress]
        if @opts[:save]
          @outf = open(@opts[:save], 'wb')
        else
          @outf = StringIO.new() # make ASCII-8BIT
        end
          
        puts "Thread Start"
        open url, uri_opts do |f|
          puts "opened #{url}"
          # everything has been download at this point.
          f.read {|chunk| @outf.write chunk}
          puts "Download.finished #{f.size}"
          #save_to_file(@opts[:save], download_data) if @opts[:save]
          @outf.close
          finish_download f
        end
      end
      @thread.join
    end
      
    def content_length_proc
      lambda do |content_length|
        download_started(content_length)
        #eval_block(@opts[:progress], self) if @opts[:progress]
      end
    end

    def progress_proc
      lambda do |size|
        # size is number of bytes xferred, so far.
        if (size - self.transferred) > (content_length / UPDATE_STEPS) #&& !@gui.busy?
          #@gui.busy = true
          @percent = (@length / size)
          eval_block(@opts[:progress], self)
          @transferred = size
        end
      end
    end

    def finish_download(f)
      @finished = true
      puts "Calling finishers #{f.size}"
      #@response = StringIO.new(download_data)

      #In case final asyncEvent didn't catch the 100%
      #@transferred = @content_length
      #Hangs here:
      #eval_block(@opts[:progress], self) if @opts[:progress]

      #:finish and block are the same
      eval_block(@blk, self) if @blk
      eval_block(@opts[:finish], self) if @opts[:finish]
    end

    def eval_block(blk, result)
      blk.call result
      # @gui.eval_block(blk, result)
    end

    def save_to_file(file_path, download_data)
      open(file_path, 'wb') { |fw| fw.print download_data }
    end

    def download_started(content_length)
      @length = content_length
      @content_length = content_length
      @percent = 0.0
      @started = true
      puts "download started #{@length} bytes"
    end

  end
end

# Monkey patch over the 'C' code. 
class Shoes::Types::App
  # Shoes::Types::App seems wrong but it works. 
  def download (url, options, &blk)
    Shoes::Download.new(url, options, &blk)
  end
end

