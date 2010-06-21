
require 'roptix_math'
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


  def mouseDown( x, y )
    @mouse      = Vector[ x, y, 0.0 ]
    @dragging   = true
    @mouse_down = mouse
  end
  

  def mouseUp( x, y )
    motion( x, y )
    
    @dragging = false
    @q_down   = q_now
  end


  def motion( x, y )
    @mouse = Vector[ x, y, 0.0 ]
    if @dragging
      from   = toSphere( mouse_down )
      to     = toSphere( mouse )
      q_drag = Quaternion.create( from, to )

      @q_now = q_drag * @q_down
      @q_now.normalize
    end
  end
end


################################################################################
#
# Class for handling mouse-driven camera manipulation
#
################################################################################

class Camera
  
  def initialize( eye, lookat, up, fov, width, width )
    @eye     = eye
    @lookat  = lookat
    @up      = up
    @fov     = fov
    @width   = width
    @height  = height
    @mouse_x = width / 2
    @mouse_y = height / 2
    @scale   = 1.0 
    @arcball = Arcball.new(  Vector[ width / 2.0, height / 2.0, 0.0 ], width / 2.0 )
  end

  def 
  def motion( button, x, y )

    dx = x - @mouse_x
    dy = y - @mouse_y

    case button
    when LEFT
      @arcball.motion( x, y )
      
    when RIGHT
      # zoom
      
    else # MIDDLE
      # translate

    end

    @mouse_x = x
    @mouse_y = y
  end


  def mouseUp( button, x, y )
    @mouse_x = x
    @mouse_y = y
    
    case button
    when LEFT
      @arcball.mouseUp( x, y )
    when RIGHT
      # zoom
    else # MIDDLE
      # translate
    end
  end

  
  def mouseDown( button, x, y )
    @mouse_x = x
    @mouse_y = y
    
    case button
    when LEFT
      @arcball.mouseDown( x, y )
    when RIGHT
      # zoom
    else # MIDDLE
      # translate
    end
  end

  def getCameraParams
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

    # update camera

    launch()

    buffer        = getOutputBuffer
    @width, @height = buffer.getSize2D()
    format        =  buffer.getFormat

    if( format != FORMAT_UNSIGNED_BYTE4 )
      raise Exception( "OptixViewer display() - requires FORMAT_UNSIGNED_BYTE4 output buffer for now" )
    end

    glDrawPixels( @width, @height, GL_BGRA, GL_UNSIGNED_BYTE, buffer.map().get() )
    buffer.unmap

    glutSwapBuffers
    
    puts @frame_count


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
  end
    
  def mouseMotion( x, y )
  end

  def resize( width, height )
  end

end

