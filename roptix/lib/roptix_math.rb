


################################################################################
#
# Misc functions
#
################################################################################

def d2r( d );   d*Math::PI/180.0   end
def r2d( r );   r*180.0/Math::PI   end



################################################################################
#
# Vector class of arbitrary length.  Move this into c exenstion with fixed size
# (eg, vec3f) if necessary.
#
################################################################################

class Vector < Array

  def Vector.create( *v )
    Vector.new( v.flatten ) # allow Vector.create( Vector[1,2,3], 4 ) 
  end

  def x(); self[0] end
  def y(); self[1] end
  def z(); self[2] end
  def w(); self[3] end

  def +@
    self
  end
  
  def -@
    Vector.new( self.map { |x| -x }  )
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
  
  def to_str
    to_s
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

  def Matrix.createFromBasis( u, v, w, t )
    Matrix.new( [ u.x, v.x, w.x, t.x,
                  u.y, v.y, w.y, t.y,
                  u.z, v.z, w.z, t.z,
                  0.0, 0.0, 0.0, 1.0 ] )
  end

  def []( i )
    @m[i]
  end
  
  def []=( i, val )
    @m[i] = val
  end

  def *( b )
    if b.is_a? Matrix
      v = Vector.new [  self[ 0] * b[ 0] + self[ 1] * b[ 4] + self[ 2] * b[ 8] + self[ 3] * b[12],
                        self[ 0] * b[ 1] + self[ 1] * b[ 5] + self[ 2] * b[ 9] + self[ 3] * b[13],
                        self[ 0] * b[ 2] + self[ 1] * b[ 6] + self[ 2] * b[10] + self[ 3] * b[14],
                        self[ 0] * b[ 3] + self[ 1] * b[ 7] + self[ 2] * b[11] + self[ 3] * b[15],
                        self[ 4] * b[ 0] + self[ 5] * b[ 4] + self[ 6] * b[ 8] + self[ 7] * b[12],
                        self[ 4] * b[ 1] + self[ 5] * b[ 5] + self[ 6] * b[ 9] + self[ 7] * b[13],
                        self[ 4] * b[ 2] + self[ 5] * b[ 6] + self[ 6] * b[10] + self[ 7] * b[14],
                        self[ 4] * b[ 3] + self[ 5] * b[ 7] + self[ 6] * b[11] + self[ 7] * b[15],
                        self[ 8] * b[ 0] + self[ 9] * b[ 4] + self[10] * b[ 8] + self[11] * b[12],
                        self[ 8] * b[ 1] + self[ 9] * b[ 5] + self[10] * b[ 9] + self[11] * b[13],
                        self[ 8] * b[ 2] + self[ 9] * b[ 6] + self[10] * b[10] + self[11] * b[14],
                        self[ 8] * b[ 3] + self[ 9] * b[ 7] + self[10] * b[11] + self[11] * b[15],
                        self[12] * b[ 0] + self[13] * b[ 4] + self[14] * b[ 8] + self[15] * b[12],
                        self[12] * b[ 1] + self[13] * b[ 5] + self[14] * b[ 9] + self[15] * b[13],
                        self[12] * b[ 2] + self[13] * b[ 6] + self[14] * b[10] + self[15] * b[14],
                        self[12] * b[ 3] + self[13] * b[ 7] + self[14] * b[11] + self[15] * b[15] ]
      return Matrix.new(v) 

    elsif b.is_a? Vector
      return Vector[ m[ 0]*b.x + m[ 1]*b.y + m[ 2]*b.z + m[ 3]*b.w,
                     m[ 4]*b.x + m[ 5]*b.y + m[ 6]*b.z + m[ 7]*b.w, 
                     m[ 8]*b.x + m[ 9]*b.y + m[10]*b.z + m[11]*b.w, 
                     m[12]*b.x + m[13]*b.y + m[14]*b.z + m[15]*b.w ]
    else
      return @m.map { |x| x*b.to_f }
    end
  end

  def determinant
    m[ 0]*m[ 5]*m[10]*m[15]-
    m[ 0]*m[ 5]*m[11]*m[14]+m[ 0]*m[ 9]*m[14]*m[ 7]-
    m[ 0]*m[ 9]*m[ 6]*m[15]+m[ 0]*m[13]*m[ 6]*m[11]-
    m[ 0]*m[13]*m[10]*m[ 7]-m[ 4]*m[ 1]*m[10]*m[15]+m[ 4]*m[ 1]*m[11]*m[14]-
    m[ 4]*m[ 9]*m[14]*m[ 3]+m[ 4]*m[ 9]*m[ 2]*m[15]-
    m[ 4]*m[13]*m[ 2]*m[11]+m[ 4]*m[13]*m[10]*m[ 3]+m[ 8]*m[ 1]*m[ 6]*m[15]-
    m[ 8]*m[ 1]*m[14]*m[ 7]+m[ 8]*m[ 5]*m[14]*m[ 3]-
    m[ 8]*m[ 5]*m[ 2]*m[15]+m[ 8]*m[13]*m[ 2]*m[ 7]-
    m[ 8]*m[13]*m[ 6]*m[ 3]-
    m[12]*m[ 1]*m[ 6]*m[11]+m[12]*m[ 1]*m[10]*m[ 7]-
    m[12]*m[ 5]*m[10]*m[ 3]+m[12]*m[ 5]*m[ 2]*m[11]-
    m[12]*m[ 9]*m[ 2]*m[ 7]+m[12]*m[ 9]*m[ 6]*m[ 3]
  end

  def inverse
    v = Array.new( 16 ) 
    d = determinant()
    v[ 0]= d*(m[ 5]*(m[10]*m[15] - m[14]*m[11]) + m[ 9]*(m[14]*m[ 7] - m[ 6]*m[15]) + m[13]*(m[ 6]*m[11] - m[10]*m[ 7]))
    v[ 4]= d*(m[ 6]*(m[ 8]*m[15] - m[12]*m[11]) + m[10]*(m[12]*m[ 7] - m[ 4]*m[15]) + m[14]*(m[ 4]*m[11] - m[ 8]*m[ 7]))
    v[ 8]= d*(m[ 7]*(m[ 8]*m[13] - m[12]*m[ 9]) + m[11]*(m[12]*m[ 5] - m[ 4]*m[13]) + m[15]*(m[ 4]*m[ 9] - m[ 8]*m[ 5]))
    v[12]= d*(m[ 4]*(m[13]*m[10] - m[ 9]*m[14]) + m[ 8]*(m[ 5]*m[14] - m[13]*m[ 6]) + m[12]*(m[ 9]*m[ 6] - m[ 5]*m[10]))
    v[ 1]= d*(m[ 9]*(m[ 2]*m[15] - m[14]*m[ 3]) + m[13]*(m[10]*m[ 3] - m[ 2]*m[11]) + m[ 1]*(m[14]*m[11] - m[10]*m[15]))
    v[ 5]= d*(m[10]*(m[ 0]*m[15] - m[12]*m[ 3]) + m[14]*(m[ 8]*m[ 3] - m[ 0]*m[11]) + m[ 2]*(m[12]*m[11] - m[ 8]*m[15]))
    v[ 9]= d*(m[11]*(m[ 0]*m[13] - m[12]*m[ 1]) + m[15]*(m[ 8]*m[ 1] - m[ 0]*m[ 9]) + m[ 3]*(m[12]*m[ 9] - m[ 8]*m[13]))
    v[13]= d*(m[ 8]*(m[13]*m[ 2] - m[ 1]*m[14]) + m[12]*(m[ 1]*m[10] - m[ 9]*m[ 2]) + m[ 0]*(m[ 9]*m[14] - m[13]*m[10]))
    v[ 2]= d*(m[13]*(m[ 2]*m[ 7] - m[ 6]*m[ 3]) + m[ 1]*(m[ 6]*m[15] - m[14]*m[ 7]) + m[ 5]*(m[14]*m[ 3] - m[ 2]*m[15]))
    v[ 6]= d*(m[14]*(m[ 0]*m[ 7] - m[ 4]*m[ 3]) + m[ 2]*(m[ 4]*m[15] - m[12]*m[ 7]) + m[ 6]*(m[12]*m[ 3] - m[ 0]*m[15]))
    v[10]= d*(m[15]*(m[ 0]*m[ 5] - m[ 4]*m[ 1]) + m[ 3]*(m[ 4]*m[13] - m[12]*m[ 5]) + m[ 7]*(m[12]*m[ 1] - m[ 0]*m[13]))
    v[14]= d*(m[12]*(m[ 5]*m[ 2] - m[ 1]*m[ 6]) + m[ 0]*(m[13]*m[ 6] - m[ 5]*m[14]) + m[ 4]*(m[ 1]*m[14] - m[13]*m[ 2]))
    v[ 3]= d*(m[ 1]*(m[10]*m[ 7] - m[ 6]*m[11]) + m[ 5]*(m[ 2]*m[11] - m[10]*m[ 3]) + m[ 9]*(m[ 6]*m[ 3] - m[ 2]*m[ 7]))
    v[ 7]= d*(m[ 2]*(m[ 8]*m[ 7] - m[ 4]*m[11]) + m[ 6]*(m[ 0]*m[11] - m[ 8]*m[ 3]) + m[10]*(m[ 4]*m[ 3] - m[ 0]*m[ 7]))
    v[11]= d*(m[ 3]*(m[ 8]*m[ 5] - m[ 4]*m[ 9]) + m[ 7]*(m[ 0]*m[ 9] - m[ 8]*m[ 1]) + m[11]*(m[ 4]*m[ 1] - m[ 0]*m[ 5]))
    v[15]= d*(m[ 0]*(m[ 5]*m[10] - m[ 9]*m[ 6]) + m[ 4]*(m[ 9]*m[ 2] - m[ 1]*m[10]) + m[ 8]*(m[ 1]*m[ 6] - m[ 5]*m[ 2]))
    return Matrix.new( v )
  end

end


################################################################################
#
# Quaternion for representing rotations
#
################################################################################
class Quaternion

  def initialize( *args )
    if args.size == 1
      @q = [ args[0][0], args[0][1], args[0][2], args[0][3] ] 
    else  
      @q = args.flatten 
    end
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
    if( args.size == 2 && args[0].is_a?( Vector ) && args[1].is_a?( Vector ) )
      # We have a to, from vector pair representing rotation
      from, to = *args 
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
  

  def /( b )
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
  
  def to_str
    to_s
  end

end


