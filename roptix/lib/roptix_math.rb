

#
# Vector class of arbitrary length.  Move this into c extension if necessary
#
class Vector
  def initialize( *args )
    raise Exception( "Vector should be of length <= 4" ) if( args.size > 4 )
    @array = args
  end

  def +@
    self
  end
  
  def -@
    self.map { |x| -x } 
  end

  def +( o )
    result = Array.new( @array.size )
    (0...@array.size).each { |i| result[i] = @array[i] + b[i] }
    result
  end

  def -( o )
    result = Array.new( @array.size )
    (0...@array.size).each { |i| result[i] = @array[i] - b[i] }
    result
  end

  def *( o )
    result = Array.new( @array.size )
    (0...@array.size) { |i| result[i] = @array[i] * o.to_f }
    result
  end

  def /( o )
    result = Array.new( @array.size )
    (0...@array.size) { |i| result[i] = @array[i] / o.to_f }
    result
  end

  def Vector.dot( a, b )
  end

  def dot( b )
  end

  def Vector.cross( a, b )
  end

  def cross( b )
  end
end


class ArcBall

  def initialize( width, height )
    @start = nil
    @end   = nil
    @adjust_width  = 1.0
    @adjust_height = 1.0
    @width  = width
    @height = height
  end

  def to_s

  end
end


