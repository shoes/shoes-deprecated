Shoes.app width: 400, height: 400 do 
        ext = "svg"
        path = "#{LIB_DIR}/snapshotfile.#{ext}"

    stack do    
        button("shoot!") do
            r = snapshot :format => ext.to_sym, :filename => path  do
                stroke blue
                strokewidth 4
                fill black
                oval 100, 100, 200
            end
            info r.inspect
            alert path, title: "snapshot saved to :"
            Shoes.show_log
        end
    end
end
