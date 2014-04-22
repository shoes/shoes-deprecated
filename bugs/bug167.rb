# Works ok with symbols instead of strings in Gtk2 (linux, win7)
# if the manual says otherwise, fix the manual.
# Won't be resolved until Shoes/OSX is working.
Shoes.app do
  radio_one = radio :group; para "one"
  radio_two = radio :group; para "two"
  radio_one.checked = true
  button "verify" do
    alert(radio_one.checked? ? "ok" : "failed") # fails
  end
end
