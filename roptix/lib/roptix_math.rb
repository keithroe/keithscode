

#
# Vector class of arbitrary length.  Move this into c exenstion with fixed size (eg, vec3f)
# if necessary
#
class Vector < Array

  def +@
    self
  end
  
  def to_ary
    self
  end

  def -@
    self.map { |x| -x } 
  end

  def +( b )
    result = Vector.new( self.size )
    (0...self.size).each { |i| result[i] = self[i] + b[i] }
    result
  end

  def -( b )
    result = Vector.new( self.size )
    (0...self.size).each { |i| result[i] = self[i] - b[i] }
    result
  end

  def *( o )
    result = Vector.new( self.size )
    (0...self.size).each { |i| result[i] = self[i] * o.to_f }
    result
  end

  def /( o )
    result = Vector.new( self.size )
    (0...self.size).each { |i| result[i] = self[i] / o.to_f }
    result
  end

  def Vector.dot( a, b )
    sum = 0.0
    (0...a.size).each do |i|
      sum += a[i] * b[i]
    end
    sum
  end

  def dot( b )
    Vector.dot( self, b )
  end

  def Vector.cross( a, b )
    raise ArgumentError( "cross requires 3D vectors" ) if a.size != 3 || b.size != 3
    Vector[ a[1]*b[2] - a[2]*b[1],
            a[2]*b[0] - a[0]*b[2],
            a[0]*b[1] - a[1]*b[0] ]
  end

  def cross( b )
    Vector.cross( self, b )
  end

  def length
    Math.sqrt( self.dot( self ) )
  end

  def normalize
    len     = self.length 
    len_inv = 1.0 / len 
    self.map! { |x| x*len_inv }
    len
  end

  def Vector.normalize( a )
    temp = a
    temp.normalize
    temp 
  end

end


def Quaternion
  def:
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


