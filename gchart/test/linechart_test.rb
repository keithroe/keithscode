
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'




chart = GChart::Chart.new
chart.legendOn 't'
chart.setChartTitle 'test1', 20, [0xff, 0, 0xff]

chart.addData [ 0.0, 0.3, 0.2, 0.5, 0.4, 0.7, 0.6, 0.9 ] do |data|
  data.name  = 'foo'
  data.scale = [ 0.0, 1.2 ]
  data.color = [ 255, 128, 128 ]
end
chart.addData [ nil, 0.1, 0.2, 0.3, 0.4, 0.3, 0.2, 0.1 ] do |data|
  data.name = 'bar'
  data.scale = [ 0.0, 1.2 ]
  data.color = [ 255, 128, 128 ]
end
chart.addData [ 0.9, 0.6, 0.4, 0.3, 0.4, 0.6, 0.9, nil ] do |data|
  data.name = 'zoo'
  data.scale = [ 0.0, 1.2 ]
  data.color = [ 255, 128, 128 ]
end

chart.addAxis( 'y' ) do |axis|
  axis.setRange( 0.0, 1.2, 0.1 )
end
chart.addAxis( 'x' ) do |axis|
  axis.setLabels( "test1", "test2", "test3" )
end

chart.png( $dir + "/pngs/test1.png" )

