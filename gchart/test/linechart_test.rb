
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'




chart = GChart::Chart.new
chart.legendOn 't'
chart.setChartTitle 'test1', 20, [0xff, 0, 0xff]

chart.addData [ 0.0, 0.3, 0.2, 0.5, 0.4, 0.7, 0.6, 0.9 ] do |data|
  data.name = 'foo'
end
chart.addData [ nil, 0.1, 0.2, 0.3, 0.4, 0.3, 0.2, 0.1 ] do |data|
  data.name = 'bar'
end
chart.addData [ 0.9, 0.6, 0.4, 0.3, 0.4, 0.6, 0.9, nil ] do |data|
  data.name = 'zoo'
end


puts chart.url
chart.png( $dir + "/pngs/test1.png" )

