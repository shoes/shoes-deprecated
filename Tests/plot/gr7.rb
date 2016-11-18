# Radar graph - 
Shoes.app width: 620, height: 480 do
  @pre_test =  [71, 35, 62, 55, 88, 76, 55] 
  @practice =  [70, 53, 83, 94, 71, 59, 82]
  @post_test = [94, 93, 96, 89, 96, 88, 93]
  @dimensions = [ ["Anger", 0, 100, "%3.0f%%"], 
               ["Contempt", 0, 100, "%3.0f%%"],
               {label: "Disgust", min: 0, max: 100, format: "%3.0f%%"},
               ["Fear", 0, 100, "%3.0f%%"],
               ["Joy", 0, 100, "%3.0f%%"],
               ["Sadness", 0, 100, "%3.0f%%"],
               ["Surprise", 0, 100, "%3.0f%%"]
             ]
  stack do
    para "Plot Radar Demo 7"
    flow do 
      button "quit" do Shoes.quit end
    end
    widget_width = 600
    widget_height = 400
    stack do
      flow do
        @grf = plot widget_width, widget_height, title: "Microexpressions Scores", 
          font: "Helvetica", auto_grid: true, grid_lines: 3, label_radius: 1.10,
          default: "skip", chart: "radar", column_settings: @dimensions
      end
    end
    @grf.add values: @pre_test, color: blue, 
      name: "Pre-Test Score", min: 0, max: 100, strokewidth: 3
    cs = app.chart_series values: @practice, color: red,
      name: "Practice Score", min: 0, max: 100, strokewidth: 3
    @grf.add cs
    @grf.add values: @post_test, color: orange,
      name: "Post-Test Score", min: 0, max: 100, strokewidth: 3
  end
end

