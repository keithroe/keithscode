
require File.dirname( __FILE__ ) + '/roptix_internal'
require File.dirname( __FILE__ ) + '/roptix_math'


def cameraParams( eye, lookat, up, vfov, aspect )

  def d2r( d );   d*Math::PI/180.0   end
  def r2d( r );   r*180.0/Math::PI   end

  hfov = r2d( 2.0*Math.atan( aspect*Math.tan( d2r(0.5*vfov ) ) ) )
  w    = lookat - eye
  flen = Math.sqrt( Vector.dot( w, w ) )
  u    = Vector.normalize( Vector.cross( w, Vector.normalize( up ) ) )
  v    = Vector.normalize( Vector.cross( u , w ) )
  ulen = flen * Math.tan( d2r( hfov*0.5 ) )
  u    = u*ulen 
  vlen = flen * Math.tan( d2r( vfov*0.5 ) )
  v    = v*vlen

  [u,v,w]
end

class UserFormat
end
