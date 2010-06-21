

################################################################################
#
# Vector class of arbitrary length.  Move this into c exenstion with fixed size (eg, vec3f)
# if necessary
#
################################################################################
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

  def to_s
    "( #{self.join( ',' )} )"
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

end


################################################################################
#
# 4x4 Matrix
#
################################################################################
class Matrix
end

