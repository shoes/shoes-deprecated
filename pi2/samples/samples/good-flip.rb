# plays with svghandles. Card flipping
# assumes svg has 4 suites of 13 card, two jokers and a back.
Shoes.app do
  @xmlstring = ''
  @suite_first = true
  @handles = []
  @pile = []
  topcard = 0 # back of deck
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
    @handles = [] # gc old ones
    @handles[0] = app.svghandle( {content: @xmlstring, group: '#back'})
    ['club', 'diamond', 'heart', 'spade'].each do |suite|
      ['1','2','3','4','5','6','7','8','9', '10','jack', 'queen', 'king'].each do |card|
        if @suite_first 
          @handles << app.svghandle( {content: @xmlstring, group: "\##{suite}_#{card}"} )
        else
          @handles << app.svghandle( {content: @xmlstring, group: "\##{card}_#{suite}"} )
        end
      end
    end
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

  #['joker_black', 'joker_red', 'back'].each do 
  #end
  @animation = nil
  @top_card = svg @handles[0], {width: 200, height: 200}
  button "tap" do
    kill_anim
    topcard = topcard + 1
    if topcard == 52
      topcard = 0
    end
    @top_card.handle = @handles[@pile[topcard]]
  end
  button "shuffle" do
    kill_anim
    temp_deck = []
    (1..52).each {|i| temp_deck[i] = i}
    @pile = shuffle_me(temp_deck)
    run_for = 0
    @animation = animate(10) do 
      r = rand(52)
      @top_card.handle = @handles[@pile[r+1]]
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
    topcard = 0
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
      topcard = 0
    end
  end
end
