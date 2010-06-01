
require 'net/http'

module GChart

  #############################################################################
  #
  # Represents a single data series
  #
  #############################################################################
  class Data
    attr_accessor :name
    attr_accessor :data
    attr_accessor :scale
    attr_accessor :color

    def initialize( data )
      @name       = nil
      @data       = data
      @scale      = nil 
      @color      = [ 128,128,128 ]
    end

    def to_s
      s  = "Data '#{name}':\n"
      s += "  data : #{data.inspect}\n"
      s += "  scale: #{scale.inspect}\n"
      s += "  color: #{color.inspect}\n"
    end
  end


  #############################################################################
  #
  # Represents a single axis graphic 
  #
  #############################################################################
  class Axis
    def initialize( which_axis )
      @position        = which_axis  # chxt
      @range           = nil         # chxr
      @labels          = nil         # chxl
      @label_positions = nil         # chxp
      @label_style     = nil         # chxs
      @tick_style      = nil         # chxtc

    end

    def setRange( range )       # chxr
      @range  = range
      @labels = nil
    end

    def setLabels( labels )     # chxl
      @labels = labels
      @range  = nil
    end

    def setLabelPositions( label_positions )
      if labels
        raise ArgumentError( "Number of label positions(%i) must match number of labels( %i )" % 
                             [ label_positions.size, @labels.size ] )
        @label_positions = label_positions
      end
    end
    
    def setLabelStyle( color, size=nil, alignment=nil, axis_or_tick=nil, tick_color=nil, format_string=nil )
      @label_style = color
      # TODO handle rest
    end
    
    def setTickStyle
    end
  end

  #############################################################################
  #
  # Chart 
  #
  #############################################################################
  class Chart


    def Chart.xAxis();  0 end
    def Chart.yAxis();  1 end
    def Chart.rAxis();  2 end
    def Chart.tAxis();  3 end

    @@url_header = 'http://chart.apis.google.com/'

    attr_accessor :size
    attr_accessor :type
    attr_accessor :data
    attr_accessor :data_type


    def initialize
      @size             = [ 512, 256 ]
      @type             = 'lc' 
      @title            = nil 
      @title_size       = nil 
      @title_color      = nil 

      @data_type        = 't' 
      @data_format      = '%.3f' 
      @data             = [] 

      @legend_on        = false
      @legend_position  = nil 
      @legend_order     = nil 

      @axes             = [] 

      @bg

      @auto_color       = true
    end

    def legendOn( position = nil, label_order = nil )
      @legend_on       = true
      @legend_position = position
      @legend_order    = label_order
    end

    def legendOff
      @legend = false 
    end

    def setChartTitle( title, size = nil, color = nil )
      @title        = title
      @title_size   = size 
      @title_color  = color 
    end

    def setDataColors( *colors )
      # If colors.size is less than number of series, colors are cycled

      if( colors.size > @data.size )
        raise ArgumentError( "Passed %i colors but only have %i data series" % [ colors.size, @data.size ] )
      end

      @data.each_with_index do |data, i|
        data.color = colors[ i % colors.size ]
      end
    end


    def axisOn( which )
      raise ArgumentError.new( "invalid axis parameter" ) if !(0...3).include?( which )
      @axes_on[ which ] = true 
    end
    def axisOff( which )
      raise ArgumentError.new( "invalid axis parameter" ) if !(0...3).include?( which )
      @axes_on[ which ] = false 
    end

    def addData( data )
      @data.push Data.new( data )
      yield @data.last if block_given? 
      @data.last
    end

    def url
      s = "#{@@url_header}/chart?cht=#{@type}&chs=#{@size.join('x')}"

      if( @title ) 
        s += "&chtt=#{@title.gsub( ' ', '+' ).gsub( "\n", '|' )}"
        if( @title_size )
          color = @title_color ? @title_color.map{ |x| "%02X" % x } : '000000'
          s += "&chts=#{color},#{@title_size}"
        end
      end

      chd  = Array.new
      chds = Array.new
      chco = Array.new
      chdl = Array.new
      min_max = [ 1e8, -1e8 ]

      # process data values
      @data.each do |data|
        puts data 
        data_vals = data.data
        min_max = data_vals.inject( min_max )  do |mm,val| 
          if( val )
            val = ( @data_format % val ).to_f
            [ mm[0] > val ? val : mm[0], mm[1] < val ? val : mm[1] ]
          else
            mm if !val
          end
        end
        data_vals.map! { |a| a == nil ? -1.0 : a }
        data_vals.map! { |data_val| @data_format % data_val.to_f }
        chd.push data_vals.join( @data_type == 't' ? ',' : '' ) 
        chds.push data.scale.join(',') if data.scale 
        chco.push data.color.map { |x| "%02X" % ( @auto_color ? rand(256) :  x ) }.join
        chdl.push data.name
      end

      # data values
      s += "&chd=#{@data_type}:"
      s += chd.join( '|' )

      # data scales
      s += '&chds='
      if chds.size > 0 
        s += chds.join( '|' )
      else
        s += min_max.join( ',' )
      end

      # data colors
      s += '&chco='
      s += chco.join( ',' )

      # data names
      if @legend_on
        s += '&chdl='
        s += chdl.join( '|' )
      end
      if @legend_position
        legend_order = @legend_order ? "|#{@legend_order}" : '' 
        s += "&chdlp=#{@legend_position}#{legend_order}"

      end

    end


    def png( out_filename)
      Net::HTTP.start("chart.apis.google.com") do |http|
        filename =  self.url.gsub( @@url_header, '' )
        puts " retrieving <#{filename}>"
        resp = http.get( filename )
        open( out_filename, "wb") do |file|
          file.write(resp.body)
        end 
      end
    end

  end



end # GChart


