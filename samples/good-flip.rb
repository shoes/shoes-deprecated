# plays with svghandles. Card flipping
# assumes svg has 4 suites of 13 card, two jokers and a back.
Shoes.app do
  @xmlstring = ''
  @suite_first = true
  @handles = [] # card handles by suite, rank. Numeric 0..52
  @pile = []    # the deck of cards (shuffled?). Numeric 0..52
  @topcard = 0  # numeric index of the handle that is currently shown in
  @top_card = nil # the on screen svg widget
  @names = []   # contains svg group ids (strings)
  
  def shuffle_me(array)
    (array.size-1).downto(1) do |i|
     j = rand(i+1)
     array[i], array[j] = array[j], array[i]
    end
    array
  end 
  
  def kill_anim
   if @animation 
     @animation.stop
     @animation.remove
     @animation = nil
   end
  end
  
  def get_handle (idx)
    # create handle when needed
    if @handles[idx] == nil
      str = @names[idx]
      puts "load: #{idx}:#{str}"
      @handles[idx] = app.svghandle ( {content: @xmlstring, group: str} )
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
      alert "Can't find order"
    end
    @handles = [] # let gc handle old ones
    idx = 1
    @handles[0] = app.svghandle( {content: @xmlstring, group: '#back'})
    @names[0] = "#back"
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
    if @topcard == 52
      @topcard = 0
    end
    @top_card.handle = get_handle @pile[@topcard]
  end
  
  if Shoes::RELEASE_TYPE =~ /TIGHT/
    fpath = "#{DIR}/samples/paris.svg"
  else
    fpath = "#{DIR}/../samples/paris.svg"
  end
  if ! File.exist? fpath 
    alert "Can't find #{fpath} - crash ahead"
  end
  init_with_path(fpath)

  @animation = nil
  flow do 
    # display back of deck at startup
    stack width: 200, height: 200 do
      @top_card = svg @handles[0], {width: 200, height: 200, click: proc { tap }  }
    end
    stack width: 100 do
      button "shuffle" do
        kill_anim
        temp_deck = []
        (1..52).each {|i| temp_deck[i] = i}
        @pile = shuffle_me(temp_deck)
        run_for = 0
        @animation = animate(10) do 
          r = rand(52)
          # @top_card.handle = @handles[@pile[r+1]]
          @top_card.handle = get_handle @pile[r+1]
          run_for = run_for + 1
          if run_for >= 20
            @animation.stop
            top_card = 0
            @top_card.handle = @handles[0]  # back of deck 
            kill_anim
          end
        end
      end
      button "reset" do
        kill_anim
        temp_deck = []
        (1..52).each {|i| temp_deck[i] = i}
        @pile = temp_deck
        @topcard = 0
        @top_card.handle = @handles[0] # back of deck
      end
      button "quit" do
        exit
      end
      button "Change Deck" do
        fpath = ask_open_file
        if fpath
          init_with_path(fpath)
          @top_card.handle = @handles[0]
          @topcard = 0
        end
      end
    end 
  end 
end
