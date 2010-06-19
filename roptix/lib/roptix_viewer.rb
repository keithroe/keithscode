
require 'rubygems'
require 'opengl'
include Gl, Glu, Glut




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

