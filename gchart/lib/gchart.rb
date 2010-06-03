
# Copyright (c) <2010> <r. keith morley>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
 
require 'net/http'

module GChart

  def yAxis(); 'y' end
  def rAxis(); 'r' end
  def tAxis(); 't' end


  #############################################################################
  #
  # Represents a single data series
  #
  #############################################################################
  class Data
    attr_accessor :name
    attr_accessor :values  # [ v1, v2, ..., vN ]
    attr_accessor :scale   # [ min, max ]
    attr_accessor :color   # [ r, g, b ] each in [0, 255]

    def initialize( values )
      @name       = nil
      @values     = values
      @scale      = nil 
      @color      = [ 128,128,128 ]
    end

    def to_s
      s  = "Data '#{name}':\n"
      s += "  data : #{values.inspect}\n"
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
    attr_accessor :position
    attr_accessor :range
    attr_accessor :labels
    attr_accessor :label_positions
    attr_accessor :label_style
    attr_accessor :title_style

    def initialize( which_axis )
      @position        = which_axis  # chxt 'x', 'y', 'r', or 't' 
      @range           = nil         # chxr [ min, max, step ]
      @labels          = nil         # chxl [ label1, label2, ..., labelN ] 
      @label_positions = nil         # chxp 
      @label_style     = nil         # chxs
      @tick_style      = nil         # chxtc

    end

    def setRange( *range )
      if range.size != 2 && range.size != 3
        raise ArgumentError.new( "setRange must take either [min, max] or [min, max, step]" )
      end
      @range  = range 
      @labels = nil
    end

    def setLabels( *labels )
      @labels = labels
      @range  = nil
    end

    def setLabelPositions( label_positions )
      if label_positions.size != @labels.size
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

  ##############################################################################
  #
  # Chart 
  #
  ##############################################################################
  class Chart

    #
    # Class for storing axes and accumulating their values into url arrays
    #
    class AxisSet
      attr_accessor :axes 

      def initialize
        @axes = []
      end

      def push( axis ); @axes.push(axis) end

      def url
        chxt = Array.new
        chxr = Array.new
        chxl = Array.new
        @axes.each_with_index do |axis, i|
          chxt.push axis.position
          chxr.push( ( [i] + axis.range ).join(',') ) if ( axis.range )
          chxl.push( [i.to_s+":"] + axis.labels )     if ( axis.labels)
        end
        s = "&chxt=#{chxt.join(',')}"
        s+= "&chxr=#{chxr.join('|')}"
        s+= "&chxl=#{chxl.join('|')}"
        s
      end
    end

    #
    # Class for storing data series and accumulating their values into url arrays
    #
    class DataSet
      attr_accessor :data
      attr_accessor :data_type
      def initialize
        @data         = Array.new
        @data_type    = 't' 
        @data_format  = "%2.3f" 
        @random_color = true 
      end

      def push( data ); @data.push( data ) end

      def url( data_format, legend_on ) 
        chd  = Array.new
        chds = Array.new
        chco = Array.new
        chdl = Array.new
        min_max = [ 1e8, -1e8 ]

        # process data values
        @data.each do |data|
          data_vals = data.values.map! { |x| x ? ( data_format % x ).to_f : nil }
          min_max = data_vals.inject( min_max )  do |mm,val| 
            !val ? mm : [ ( mm[0] > val ? val : mm[0] ), ( mm[1] < val ? val : mm[1] ) ]
          end
          #TODO: base this on scale, not hardcoded to -1.0
          data_vals.map! { |a| a == nil ? -1.0 : a }
          chd.push  data_vals.join( @data_type == 't' ? ',' : '' ) 
          chds.push data.scale if data.scale 
          chco.push data.color.map { |x| "%02X" %  x }.join
          chdl.push data.name
        end

        # data values
        s  = "&chd=#{@data_type}:#{chd.join( '|' )}"

        # data scales
        s += "&chds=#{(chds.size > 0 ? chds : min_max).join( ',' )}"

        # data colors
        s += "&chco=#{chco.join( ',' )}"

        # data names
        s += "&chdl=#{chdl.join( '|' )}" if legend_on
        s
      end
    end

    
    #
    # Chart class implementation 
    #

    @@server = 'http://chart.apis.google.com/'

    attr_accessor :size
    attr_accessor :type
    attr_accessor :data


    def initialize
      @size             = [ 512, 256 ]
      @type             = 'lc' 
      @title            = nil 
      @title_size       = nil 
      @title_color      = nil 


      @legend_on        = false
      @legend_position  = nil 
      @legend_order     = nil 

      @axis_set         = AxisSet.new 
      @data_set         = DataSet.new 

      @bg

      @data_format      = "%.3f"
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
      
      if( colors.size > @data_set.data.size )
        raise ArgumentError( "Passed %i colors but only have %i data series" % [ colors.size, @data.size ] )
      end

      @data_set.data.each_with_index do |data, i|
        data.color = colors[ i % colors.size ]
      end
    end


    def addData( data )
      @data_set.push( Data.new( data ) )
      yield @data_set.data.last if block_given? 
      @data_set.data.last
    end


    def addAxis( which )
      raise ArgumentError.new( "invalid axis parameter" ) if ![ 'x', 'y', 'r', 't' ].include?( which )
      @axis_set.push Axis.new( which )
      yield @axis_set.axes.last if block_given? 
      @axis_set.axes.last
    end


    def url
      s = "#{@@server}/chart?cht=#{@type}&chs=#{@size.join('x')}"

      if( @title ) 
        s += "&chtt=#{@title.gsub( ' ', '+' ).gsub( "\n", '|' )}"
        if( @title_size )
          color = @title_color ? @title_color.map{ |x| "%02X" % x } : '000000'
          s += "&chts=#{color},#{@title_size}"
        end
      end

      if @legend_position
        legend_order = @legend_order ? "|#{@legend_order}" : '' 
        s += "&chdlp=#{@legend_position}#{legend_order}"

      end

      s += @data_set.url( @data_format, @legend_on )
      s += @axis_set.url()
      s
    end


    def png( out_filename)
      Net::HTTP.start("chart.apis.google.com") do |http|
        filename =  self.url.gsub( @@server, '' )
        resp = http.get( filename )
        open( out_filename, "wb") do |file|
          file.write(resp.body)
        end 
      end
    end

  end

end # GChart

