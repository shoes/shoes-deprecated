# plays with svghandles. Card flipping
# assumes svg has 4 suites of 13 card, two jokers and a back.
Shoes.app do
  if Shoes::RELEASE_TYPE =~ /TIGHT/
    fpath = "#{DIR}/samples/paris.svg"
  else
    fpath = "#{DIR}/../samples/paris.svg"
  end
  if ! File.exist? fpath 
    alert "Can't find #{fpath} - crash ahead"
  end
  fl = File.open(fpath,"r");
  xmlstring = fl.read
  puts "#{xmlstring.size}"
  fl.close
  handles = []
  pile = []
  topcard = 0 # back of deck
  handles[0] = app.svghandle( {content: xmlstring, group: '#back'})
  # build handles (I know the names)
  ['club', 'diamond', 'heart', 'spade'].each do |suite|
     ['1','2','3','4','5','6','7','8','9', '10','jack', 'queen', 'king'].each do |card|
       handles << app.svghandle( {content: xmlstring, group: "\##{suite}_#{card}"} )
     end
  end
  #['joker_black', 'joker_red', 'back'].each do 
  #end
  @top_card = svg handles[0], {width: 200, height: 200}
  button "tap" do
    topcard = topcard + 1
    if topcard == 52
      topcard = 0
    end
    @top_card.handle = handles[topcard]
  end
  button "shuffle" do
    temp_deck = []
    (1..52).each do
    end
  end
end
