Shoes.app do
    w = 30
    stack do
        flow do
            para "text : "
            @author = edit_line "", :width => w
            para "long text to see what happens : ", margin: [0,20,0,0]
        end
        flow do
            para "text : "
            line 40,5,40+w,5, strokewidth: 5, stroke: blue
            inscription "edit_line and blue line width is #{w} pixels", margin_left: w+5
        end
    end
    w = 100
    stack do
        flow do
            para "text : "
            @author = edit_line "", :width => w
            para "long text to see what happens : ", margin: [0,20,0,0]
        end
        flow do
            para "text : "
            line 40,5,40+w,5, strokewidth: 5, stroke: blue
            inscription "edit_line and blue line width is #{w} pixels", margin_left: w+5
        end
    end 
    w = 165
    stack do
        flow do
            para "text : "
            @author = edit_line "", :width => w
            para "long text to see what happens : ", margin: [0,20,0,0]
        end
        flow do
            para "text : "
            line 40,5,40+w,5, strokewidth: 5, stroke: blue
            inscription "edit_line and blue line width is #{w} pixels", margin_left: w+5
        end
    end     
    w = 220
    stack do
        flow do
            para "text : "
            @author = edit_line "", :width => w
            para "long text to see what happens : ", margin: [0,20,0,0]
        end
        flow do
            para "text : "
            line 40,5,40+w,5, strokewidth: 5, stroke: blue
            inscription "edit_line and blue line width is #{w} pixels", margin_left: w+5
        end
    end
end
