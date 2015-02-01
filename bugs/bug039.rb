# can't go back in Cobbler.
Shoes.app {
   button("splash")  { Shoes.splash and close }
   button("manual")  { Shoes.show_manual and close }
   button("console")  { Shoes.show_log and close }
   button("file")  { Shoes.show_selector and close }
   button("cobbler")  { Shoes.cobbler and close }
   button("package")  { Shoes.app_package and close }
   button("shy")  { Shoes.package_app and close }
}
