# plays with svghandles. Card flipping
# assumes svg has 4 suites of 13 card, two jokers and a back.
Shoes.app do
  @xmlstring = ''
  @suite_first = true
  @handles = [] # card handles by suite, rank. Numeric 0..52
  @pile = []    # the deck of cards (shuffled?). Numeric 0..52
  @topcard = 0  # numeric index of the handle that is currently shown in
  @top_card = nil # the on screen svg widget so it has widget methods
  @names = []   # contains svg group ids (strings)

  def kill_anim
   if @animation
     @animation.stop
     @animation.remove
     @animation = nil
   end
  end

  def get_handle (idx)
    # create handle when needed
    #puts "query #{idx}"
    if @handles[idx] == nil
      str = @names[idx]
      #puts "load: #{idx}:#{str}"
      han = app.svghandle ( {content: @xmlstring, group: str} )
      @handles[idx] = han
    end
    @handles[idx]
  end

  def init_with_path(path)
    File.open(path) {|f| @xmlstring = f.read}
    (1..52).each {|i| @pile[i] = i}
    full_deck = app.svghandle( {content: @xmlstring} )
    if full_deck.group? '#club_1'
      @suite_first = true
    elsif full_deck.group? "#1_club"
      @suite_first = false
    else
      debug "Can't find order"
    end
    @handles = [] # let gc handle old ones
    @handles[0] = app.svghandle( {content: @xmlstring, group: '#back'})
    @names[0] = "#back"
    #@handles[0] = get_handle(0);
    idx = 1
    ['club', 'diamond', 'heart', 'spade'].each do |suite|
      ['1','2','3','4','5','6','7','8','9', '10','jack', 'queen', 'king'].each do |card|
        @names[idx] = @suite_first ? "\##{suite}_#{card}" : "\##{card}_#{suite}"
        idx = idx + 1
      end
    end
  end

  def tap
    kill_anim
    @topcard = @topcard + 1
    if @topcard >= 53
      @topcard = 0
    end
    han = get_handle @pile[@topcard]
    @top_card.handle = han
end

  # finding script resources is odd when developing a samples/myprogram.rb
  # particularly on osx when invoked with ./cshoes mydir/myprogram.rb
  # DIR is not sufficient.
  # its best to return a full path.
  def find_paris
    cdir = Dir.getwd
    if File.exist? "paris.svg"
      "#{cdir}/paris.svg"
    elsif File.exist? "#{DIR}/paris.svg"
      "#{DIR}/paris.svg"
    else
      ask_open_file "Locate paris.svg"
    end
  end

  fpath = find_paris
  if ! File.exist? fpath
    alert "Can't find #{fpath} - crash ahead"
  end
  init_with_path(fpath)

  @animation = nil
  flow do
    # display back of deck at startup
    stack width: 180, height: 270 do
      @backgrd = background orange, width: 181, height: 270, margin: 8, curve: 10
      han = get_handle(0) #back of deck  
      @top_card = svg han, {width: 160, height: 250, margin: 10, aspect: false, click: proc { tap }  }
    end
    stack width: 100 do
      button "shuffle" do
        kill_anim
        temp_deck = []
        (1..52).each {|i| temp_deck[i-1] = i}
        @pile = temp_deck.shuffle
        @pile.unshift( 0)
        run_for = 0
        @animation = animate(10) do
          r = rand(52)
          t = r+1
          @top_card.handle = get_handle @pile[r+1]
          run_for = run_for + 1
          if run_for >= 20
            @animation.stop
            top_card = 0
            @top_card.handle = get_handle(0)  # back of deck
            kill_anim
          end
        end
      end
      button "reset" do
        kill_anim
        temp_deck = []
        (1..52).each {|i| temp_deck[i] = i}
        @pile = temp_deck
        @pile[0] = 0
        @topcard = 0
        @top_card.handle = get_handle(0) # back of deck
      end
      button "quit" do
        Shoes.exit
      end
      button "Change Deck" do
        fpath = ask_open_file
        if fpath
          init_with_path(fpath)
          @top_card.handle = get_handle(0)
          @topcard = 0
        end
      end
    end
  end
end
