
require File.dirname( __FILE__ ) + '/roptix_internal'


def cross( a, b )
  raise ArgumentError( "cross requires 3D vectors" ) if a.length != 3 || b.length != 3
  [ a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0] ]
end

def dot( a, b )
  sum = 0.0
  (0...a.size).each do |i| 
    sum += a[i] * b[i]
  end
  sum
end

def subtract( a, b )
  result = Array.new 
  (0...a.size).each do |i| 
    result[i] = a[i] - b[i]
  end
  result 
end

def normalize( v )
  inv_len = 1.0 / Math.sqrt( dot(v,v) )
  v.map { |x| x*inv_len }
end


def cameraParams( eye, lookat, up, vfov, aspect )

  def d2r( d );   d*Math::PI/180.0   end
  def r2d( r );   r*180.0/Math::PI   end

  hfov = r2d( 2.0*Math.atan( aspect*Math.tan( d2r(0.5*vfov ) ) ) )
  w    = subtract( lookat, eye )
  flen = Math.sqrt( dot( w, w ) )
  u    = normalize( cross( w, normalize( up ) ) )
  v    = normalize( cross( u , w ) )
  ulen = flen * Math.tan( d2r( hfov*0.5 ) )
  u    = u.map { |x| x* ulen }
  vlen = flen * Math.tan( d2r( vfov*0.5 ) )
  v    = v.map { |x| x* vlen }

  [u,v,w]
end

class UserFormat
end
