

################################################################################
#
# Vector class of arbitrary length.  Move this into c exenstion with fixed size
# (eg, vec3f) if necessary.
#
################################################################################

class Vector < Array

  def x(); self[0] end
  def y(); self[1] end
  def z(); self[2] end
  def w(); self[3] end

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

  def to_s
    "( #{self.join( ',' )} )"
  end

end


################################################################################ #
# 4x4 Matrix
#
################################################################################
class Matrix
  attr_accessor :m
  def initialize( m = nil )
    if m 
      @m = m
    else
      @m = [ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 ]
    end
  end

  def []( i )
    @m[i]
  end
  
  def []=( i, val )
    @m[i] = val
  end

  def *( b )
    if b.is_a? Matrix
      # TODO
    elsif b.is_a? Vector
      return Vector[ m[ 0]*b.x + m[ 1]*b.y + m[ 2]*b.z + m[ 3]*b.w,
                     m[ 4]*b.x + m[ 5]*b.y + m[ 6]*b.z + m[ 7]*b.w, 
                     m[ 8]*b.x + m[ 9]*b.y + m[10]*b.z + m[11]*b.w, 
                     m[12]*b.x + m[13]*b.y + m[14]*b.z + m[15]*b.w ]
    else
      return @m.map { |x| x*b.to_f }
    end
  end
end


################################################################################
#
# Quaternion for representing rotations
#
################################################################################
def Quaternion

  def initialize( w, x, y, z )
    @q = [ w, x, y, z ]
  end

  def []( i )
    @q[i]
  end
  
  def []=( i, val )
    @q[i] = val
  end


  def w(); @q[0] end
  def x(); @q[1] end
  def y(); @q[2] end
  def z(); @q[3] end

  def Quaternion.create( *args )
    if args.size == 2 && args[0].is_a? Vector && args[1].is_a? Vector
      # We have a to, from vector pair representing rotation
      from, to = *args 
      puts "creating quat from #{from} -> #{to}"
      c = Vector.cross( from, to )
      w = Vector.dot( from, to )
      x = c[0]
      y = c[1]
      z = c[2]
    
      return Quaternion.new( w, x, y, z )
    else
      raise Exception.new "Quaternion.create called with bad params <<#{args.inspect}>>"
    end
  end


  def *( b )
    if( b.is_a? Quaternion )
      w = @q[0]*b[0] - @q[1]*b[1] - @q[2]*b[2] - @q[3]*b[3]
      x = @q[0]*b[1] + @q[1]*b[0] + @q[2]*b[3] - @q[3]*b[2]
      y = @q[0]*b[2] + @q[2]*b[0] + @q[3]*b[1] - @q[1]*b[3]
      z = @q[0]*b[3] + @q[3]*b[0] + @q[1]*b[2] - @q[2]*b[1]

      return Quaternion.new( w, x, y, z )
    else
      f = b.to_f
      return Quaternion.new( f*@q[0], f*@q[1], f*@q[2], f*@q[3] )
    end
  end
  

  def *( b )
    f = b.to_f
    return Quaternion.new( @q[0]/f, @q[1]/f, @q[2]/f, @q[3]/f )
  end

  def +( b )
    return Quaternion.new( @q[0]+b[0], @q[1]+b[1], @q[2]+b[2], @q[3]+b[3] )
  end
  
  def -( b )
    return Quaternion.new( @q[0]-b[0], @q[1]-b[1], @q[2]-b[2], @q[3]-b[3] )
  end


  # l2 norm
  def norm
    Math.sqrt( @q[0]*@q[0] + @q[1]*@q[1] + @q[2]*@q[2] + @q[3]*@q[3] )
  end

  def normalize
    n     = self.norm
    n_inv = 1.0 / n
    @q[0] = @q[0] * n_inv
    @q[1] = @q[1] * n_inv
    @q[2] = @q[2] * n_inv
    @q[3] = @q[3] * n_inv
    n
  end

  def getRotation()
    v = Quaternion.new( *@q )
    v.normalize
    axis  = Vector[ @q[1], @q[2], @q[2] ]
    angle = 180.0 / Math::PI  * 2.0 * Math.acos( @q[0] )
    return [ axis, angle ]
  end

  def getMatrix
    m = Array.new
    m[0] = 1.0 - 2.0*y*y - 2.0*z*z
    m[1] = 2.0*x*y + 2.0*z*w
    m[2] = 2.0*x*z - 2.0*y*w
    m[3] = 0.0

    m[4] = 2.0*x*y - 2.0*z*w
    m[5] = 1.0 - 2.0*x*x - 2.0*z*z
    m[6] = 2.0*z*y + 2.0*x*w
    m[7] = 0.0

    m[8] = 2.0*x*z + 2.0*y*w
    m[9] = 2.0*z*y - 2.0*x*w
    m[10] = 1.0 - 2.0*x*x - 2.0*y*y
    m[11] = 0.0

    m[12] = 0.0
    m[13] = 0.0
    m[14] = 0.0
    m[15] = 1.0

    Matrix.new( m )
  end


  def to_s
    "[ #{@q[0]}, ( #{@q[1]}, #{@q[2]}, #{@q[3]} ) ]"
  end

end


