
require File.dirname( __FILE__ ) + '/roptix_math'
require 'time'
require 'rubygems'
require 'opengl'
include Gl, Glu, Glut



################################################################################
#
# Arcball rotation controller
#
################################################################################
class Arcball
  def initialize( location, radius )
    @location   = location
    @radius     = radius
    @mouse      = Vector[ 0.0, 0.0, 0.0 ]
    @mouse_down = Vector[ 0.0, 0.0, 0.0 ]
    @dragging   = false
    @q_down     = Quaternion.new( 1.0, 0.0, 0.0, 0.0 )
    @q_now      = Quaternion.new( 1.0, 0.0, 0.0, 0.0 )
  end


  def reset( location, radius )
    @location   = location
    @radius     = radius
  end


  def toSphere( v )
    sphere = Vector[ ( v[0] - @location[0] ) / @radius,
                     ( v[1] - @location[1] ) / @radius,
                     0.0 ]
    magnitude = sphere.length
    if magnitude > 1.0
      sphere = sphere / Math.sqrt( magnitude )
    else
      sphere[2] = Math.sqrt( 1.0 - magnitude )
    end

    sphere
  end

  def getMatrix
    return @q_now.getMatrix
  end

  def mouseDown( x, y )
    @mouse      = Vector[ x, y, 0.0 ]
    @dragging   = true
    @mouse_down = Vector.new( @mouse )
  end
  

  def mouseUp( x, y )
    motion( x, y )
    
    @dragging = false
    @q_down   = Quaternion.new( @q_now )
  end


  def motion( x, y )
    @mouse = Vector[ x, y, 0.0 ]
    if @dragging
      from   = toSphere( @mouse_down )
      to     = toSphere( @mouse )
      q_drag = Quaternion.create( from, to )

      @q_now = q_drag * @q_down
      @q_now.normalize
    end
    @mouse_down = Vector.new( @mouse )
  end
end


################################################################################
#
# Class for handling mouse-driven camera manipulation
#
################################################################################

class Camera
  
  def initialize( eye, lookat, up, fov, width, height )
    @eye     = eye
    @lookat  = lookat
    @up      = up
    @vfov    = fov
    @width   = width
    @height  = height

    @mouse_x = width / 2
    @mouse_y = height / 2
    @button  = GLUT_LEFT_BUTTON

    @scale   = 1.0 
    @arcball = Arcball.new(  Vector[ width / 2.0, height / 2.0, 0.0 ], width / 2.0 )
  end

  def windowResized( width, height )
    @width  = width
    @height = height
    @arcball.reset( Vector[ width / 2.0, height / 2.0, 0.0 ], width / 2.0 )
  end


  def motion( x, y )
    dx = x - @mouse_x
    dy = y - @mouse_y

    case @button
    when GLUT_LEFT_BUTTON
      @arcball.motion( x, y )
      transform( @arcball.getMatrix )
    when GLUT_RIGHT_BUTTON
      scale = -dy.to_f / @height.to_f
      scale = scale * 5.0 # dolly speed
      d = (@lookat - @eye) * scale
      @eye = @eye + d

    else # GLUT_MIDDLE_BUTTON
      # translate
    end

    @mouse_x = x
    @mouse_y = y
  end


  def mouseUp( button, x, y )
    @mouse_x = x
    @mouse_y = y
    
    case button
    when GLUT_LEFT_BUTTON
      @arcball.mouseUp( x, y )
      transform( @arcball.getMatrix )
    when GLUT_RIGHT_BUTTON
      # zoom
    else # GLUT_MIDDLE_BUTTON
      # translate
    end
  end

  
  def mouseDown( button, x, y )
    @mouse_x = x
    @mouse_y = y
    @button  = button
    
    case button
    when GLUT_LEFT_BUTTON
      @arcball.mouseDown( x, y )
      transform( @arcball.getMatrix )
    when GLUT_RIGHT_BUTTON
      # zoom
    else # GLUT_MIDDLE_BUTTON
      # translate
    end
  end


  def transform( xform )
    eye, u, v, w = self.params 
    frame  = Matrix.createFromBasis( Vector.normalize( u ),
                                     Vector.normalize( v ),
                                     Vector.normalize( -w ),
                                     @lookat )

    frame_inv = frame.inverse

    final_trans  = frame * xform * frame_inv
    up4          = Vector.create( @up,     0.0 ) 
    eye4         = Vector.create( @eye,    1.0 )  
    lookat4      = Vector.create( @lookat, 1.0 )

    @up     = ( final_trans*up4 )[ 0..2 ]
    @eye    = ( final_trans*eye4 )[ 0..2 ]
    @lookat = ( final_trans*lookat4 )[ 0..2 ]
  end

  
  def params()
    aspect  = @width.to_f / @height.to_f
    hfov    = r2d( 2.0*Math.atan( aspect*Math.tan( d2r(0.5*@vfov ) ) ) )

    w    = @lookat - @eye
    flen = Math.sqrt( Vector.dot( w, w ) )
    u    = Vector.normalize( Vector.cross( w, Vector.normalize( @up ) ) )
    v    = Vector.normalize( Vector.cross( u , w ) )
    ulen = flen * Math.tan( d2r(  hfov*0.5 ) )
    u    = u*ulen
    vlen = flen * Math.tan( d2r( @vfov*0.5 ) )
    v    = v*vlen

    [ @eye, u, v, w ]
  end

end



################################################################################
#
# Viewer class to be subclassed by applications
#
################################################################################
class OptixViewer

  def initialize
    @frame_count = 0
    @width  = 512
    @height = 512

    @last_fps_time  = 0.0
    @last_fps_frame = 0
    @last_fps_text  = ''
    @display_fps    = true

    @camera        = nil  #
    @context       = nil  # subclass needs to initialize these params
    @output_buffer = nil  #
  end

  def run
    initScene

    glutInit
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE )
    glutInitWindowSize( 1, 1 )
    glutInitWindowPosition(100,100)
    glutCreateWindow( "test" )
    glutHideWindow()
    glutReshapeWindow( @width, @height )

    # Initialize state
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, @width, @height);

    glutShowWindow();

    # Set callbacks
    glutKeyboardFunc( self.method( 'keyPressed').to_proc )
    glutDisplayFunc( self.method( 'display' ).to_proc )
    glutMouseFunc( self.method( 'mouseButton' ).to_proc )
    glutMotionFunc( self.method( 'mouseMotion' ).to_proc )
    glutReshapeFunc( self.method( 'resize' ).to_proc )
    glutIdleFunc( self.method( 'idle' ).to_proc )

    glutMainLoop
  end

  def drawText( text, x, y, font )
    # Save state
    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT )

    glDisable( GL_TEXTURE_2D )
    glDisable( GL_LIGHTING )
    glDisable( GL_DEPTH_TEST)

    shadow_color = Vector[ 0.05, 0.05, 0.05 ]
    text_color   = Vector[ 0.95, 0.95, 0.95 ]

    # Shift shadow one pixel to the lower right.
    glColor3f( *shadow_color )
    glWindowPos2f(x + 1.0, y - 1.0)
    text.each_byte { |c| glutBitmapCharacter( font, c ) }

    # main text
    glColor3f( *text_color )
    glWindowPos2f(x, y)
    text.each_byte { |c| glutBitmapCharacter( font, c ) }

    # Restore state
    glPopAttrib()
  end

  def OptixViewer.createOutputBuffer( format, width, height, use_vbo = true )
    if( use_vbo )
      #Allocate first the memory for the gl buffer, then attach it to OptiX.
      vbo = glGenBuffers( 1 )
      glBindBuffer( GL_ARRAY_BUFFER, vbo )
      glBufferData( GL_ARRAY_BUFFER, getElementSize( format ) * width * height, 0, GL_STREAM_DRAW )
      glBindBuffer( GL_ARRAY_BUFFER, 0 )

      buffer = @context.createBufferFromGLBO( RT_BUFFER_OUTPUT, vbo )
      buffer.setFormat(format)
      buffer.setSize( width, height )
    else
      buffer = context.createBuffer( RT_BUFFER_OUTPUT, format, width, height )
    end

  end



  #-----------------------------------------------------------------------------
  # 'virtual' interface
  #-----------------------------------------------------------------------------
  def initScene
  end

  def launch
  end

  def getOutputBuffer
  end
  

  #-----------------------------------------------------------------------------
  # Glut callbacks
  #-----------------------------------------------------------------------------
  def idle
    glutPostRedisplay()
  end


  def display()

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    eye, u, v, w = @camera.params
    @context.queryVariable( "eye" ).set3f( *eye ) 
    @context.queryVariable(   "U" ).set3f( *u )
    @context.queryVariable(   "V" ).set3f( *v )
    @context.queryVariable(   "W" ).set3f( *w )

    launch()

    @width, @height = @output_buffer.getSize2D()
    format          = @output_buffer.getFormat

    if( format != FORMAT_UNSIGNED_BYTE4 )
      raise Exception( "OptixViewer display() - requires FORMAT_UNSIGNED_BYTE4 output buffer for now" )
    end

    glDrawPixels( @width, @height, GL_BGRA, GL_UNSIGNED_BYTE, @output_buffer.map().get() )
    @output_buffer.unmap

    if( @frame_count > 0 && @display_fps )
      current_time = Time.now.to_f 
      dt = current_time - @last_fps_time
      if( dt > 0.25 ) 
        @last_fps_text  = "fps: %7.2f" % [ (@frame_count - @last_fps_frame) / dt ]
        @last_fps_time  = current_time 
        @last_fps_frame = @frame_count 
      end
      drawText( @last_fps_text, 10.0, 10.0, GLUT_BITMAP_8_BY_13 )
    end

    glutSwapBuffers
    
    @frame_count += 1
  end


  def keyPressed( key, x, y )
    case key
    when 'Q'[0], 'q'[0], 27 # ESC
      exit( 0 )
    else
      puts "got key #{key} at #{x}, #{y}"
    end
  end


  def mouseButton( button, state, x, y)
    if( state == GLUT_UP )
      @camera.mouseUp( button, x, @width - y - 1 )
    else
      @camera.mouseDown( button, x, @width - y - 1 )
    end

    glutPostRedisplay
  end
    

  def mouseMotion( x, y )
    @camera.motion( x, @width - y - 1 )
  end


  def resize( width, height )
    @width  = width
    @height = height
    @camera.windowResized( @width, @height )
    @output_buffer.setSize2D( width, height )
  end

end

