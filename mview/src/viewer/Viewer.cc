
#include <mesh/Mesh.h>
#include <viewer/Viewer.h>
#include <math/Vector3.h>
#include <GLUT/glut.h>
#include <pthread.h>
#include <sstream>

// define static members
Viewer* Viewer::_instance = NULL;


Viewer::Viewer() : _nx(500), _ny(500), _shade_mode(FLAT), _button(Camera::LEFT)
{ }


void Viewer::view( int argc, char** argv )
{
    _timer.reset();
    _bbox = _meshes[0]->getBBox();
    for (MeshList::iterator itr = _meshes.begin(); itr != _meshes.end(); itr++ )
        _bbox = BBox::surround( (*itr)->getBBox(), _bbox );

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize( _nx, _ny );
    _camera.setResolution( _nx, _ny );
    _camera.setSceneBBox( _bbox );
    glutInitWindowPosition(100,100);
    _main_window = glutCreateWindow(" MView - A simple mesh visualization tool");

    init();

    glutMouseFunc(mouse);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); 
    glutIdleFunc(idle);
    glutMotionFunc(motion);
    glutReshapeFunc( reshape );

    glutMainLoop();

    return;
}


// TODO: Look into that recent wireframe paper
void Viewer::display()
{
    Viewer& view = Viewer::instance();

    // projection matrix transform
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    view.camera().applyProjection();
   
    // view matrix (camera) transform
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    view.camera().applyView();
    view.camera().applyModel();
    
    // draw the mesh
    glClearColor(.2, .2, .2, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    MeshList::iterator itr;
    if ( view._shade_mode == FLAT ) {
        for ( itr = view._meshes.begin(); itr != view._meshes.end(); ++itr ) {
            (*itr)->draw( Mesh::FACE );
        }
    } else if ( view._shade_mode == SMOOTH ) {
        for ( itr = view._meshes.begin(); itr != view._meshes.end(); ++itr ) {
            (*itr)->draw( Mesh::VERTEX);
        }
    } else if ( view._shade_mode == LINE ) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for ( itr = view._meshes.begin(); itr != view._meshes.end(); ++itr ) {
            (*itr)->draw( Mesh::NONE );
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else  { 
        //  _shade_mode == HIDDENLINE 

        // draw the polygons into the depth buffer
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1, 1);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        for ( itr = view._meshes.begin(); itr != view._meshes.end(); ++itr ) {
            (*itr)->draw( Mesh::NONE );
        }

        // now draw lines into the color buffer
        glDisable(GL_POLYGON_OFFSET_FILL);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for ( itr = view._meshes.begin(); itr != view._meshes.end(); ++itr ) {
            (*itr)->draw( Mesh::NONE );
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // print stats
    static char mssg[128];
    float ms = static_cast<float>( view.timer().getMicroReset() );
    sprintf( mssg, "fps: %3.2f", 1000000.0f / ms );
    printString( mssg, 10, 10 );
    
    // draw the scene
    glutSwapBuffers();
    
   }


void Viewer::cycleShadeMode() 
{
    setShadeMode( ( shadeMode() + 1 ) % 4 );
    if ( shadeMode()  == FLAT ) {
        glShadeModel( GL_FLAT );
        glEnable( GL_LIGHT0 );
        GLfloat mat_emission[]  = { 0.0, 0.0, 0.0, 1.0 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission) ;
    } else if ( shadeMode()  == SMOOTH ) {
        glShadeModel( GL_SMOOTH );
        glEnable( GL_LIGHT0 );
    } else if ( shadeMode()  == LINE ) {
        glShadeModel( GL_FLAT );
        glDisable( GL_LIGHT0 );
        GLfloat mat_emission[]  = { 0.7, 0.7, 0.7, 1.0 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission) ;

    } else {  // ( shadeMode()  == LINE ) 
        glShadeModel( GL_FLAT );
        glDisable( GL_LIGHT0 );
        GLfloat mat_emission[]  = { 0.7, 0.7, 0.7, 1.0 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission) ;
    }

}


void Viewer::init()
{
    Viewer& viewer = Viewer::instance();

    // setup camera 



    // camera transform
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    viewer.camera().applyProjection();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    viewer.camera().applyView();

    
    // set up material model
    GLfloat mat_diffuse[]   = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat mat_spec[]      = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat mat_amb[]       = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat mat_shine[]     = { 50.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shine);

    // set up lights 
    GLfloat light1_pos[]    = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat light1_color[]  = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat ambient_color[]  = { 0.05, 0.05, 0.05, 1.0 };

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightfv(GL_LIGHT0, GL_POSITION,  light1_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light1_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light1_color);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glShadeModel(GL_FLAT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_color);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // initialize gl
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    glEnable( GL_NORMALIZE );
    glColor3f(1, 1, 1);
}


void Viewer::idle()
{
    Viewer& viewer = Viewer::instance();

    if (glutGetWindow() != viewer._main_window)
        glutSetWindow( viewer._main_window);

    glutPostRedisplay();
}  


void Viewer::reshape(int  x, int y)
{  
    Viewer::instance().camera().setResolution( x, y );
    Viewer::instance().setResolution( x, y );
    glViewport( 0, 0, x, y );
    glutPostRedisplay();
}


void Viewer::keyboard(unsigned char key, int x, int y)
{
    Viewer& viewer = Viewer::instance();
    viewer.invertMouse( x, y );

    switch(key) {
        case ' ':
            viewer.cycleShadeMode();
            break;
        case  27:
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}


void Viewer::mouse(int button, int state, int x, int y)
{
    Viewer& viewer = Viewer::instance();
    viewer.invertMouse( x, y );
    viewer.setButton( button );
    if (state==GLUT_UP)
        viewer.camera().mouseUp( Camera::MouseButton(button), x, y );
    else
        viewer.camera().mouseDown( Camera::MouseButton(button), x, y );

    
    glutPostRedisplay();
}


void Viewer::motion(int x, int y)
{
    Viewer& viewer = Viewer::instance();
    viewer.invertMouse( x, y );
    viewer.camera().motion( viewer._button, x, y );
}

void Viewer::printString( const char* s, int x, int y, void* font )
{
    Viewer& viewer = Viewer::instance();

    std::string str( s );
    // set up projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewer.nx(), 0, viewer.ny(), -1, 1);

    // set up modelview 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
   
    glDisable( GL_LIGHTING ); 
    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2i(x, y);
    for ( std::string::iterator it = str.begin(); it != str.end(); ++it )
        glutBitmapCharacter( font, *it );
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable( GL_LIGHTING ); 
    
    // restore modelview 
    glPopMatrix();
    
    // restore projection 
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW );


    /*
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i( x, y );

    for ( std::string::iterator it = str.begin(); it != str.end(); ++it )
        glutBitmapCharacter( font, *it );
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW);
    */
} 


