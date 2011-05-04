
#include <GL/glut.h>
#include <iostream>
#include <vector>
//#include <sstream>
#include <iterator>




std::vector<float> samples;

int grid_divisions = 8;


void displayGrid() 
{
    glColor3f( 0.9, 0.9, 0.9 );
    double step = 1.0 / static_cast<double>( grid_divisions*grid_divisions );
    glBegin(GL_LINES);
    for( int i = 0; i < grid_divisions*grid_divisions; ++i ) 
    {
        double pos = static_cast<double>( i )*step;
        glVertex3f( pos, 0.0, 0.0);
        glVertex3f( pos, 1.0, 0.0);
        glVertex3f( 0.0, pos, 0.0);
        glVertex3f( 1.0, pos, 0.0);
    }
    glEnd();
    
    
    glColor3f( 0.6, 0.6, 0.6 );

    step = 1.0 / static_cast<double>( grid_divisions );
    glBegin(GL_LINES);
    for( int i = 0; i < grid_divisions; ++i ) 
    {
        double pos = static_cast<double>( i )*step;
        glVertex3f( pos, 0.0, 0.0);
        glVertex3f( pos, 1.0, 0.0);
        glVertex3f( 0.0, pos, 0.0);
        glVertex3f( 1.0, pos, 0.0);
    }
    glEnd();

}


void displayPoints()
{
    glColor3f( 0.5, 0.0, 0.0 );
    glPointSize( 3 );
    glBegin( GL_POINTS );
    for( int i = 0; i < samples.size() / 2; ++i )
    {
        glVertex2f( samples[i*2+0], samples[i*2+1] );
    }
    glEnd();
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT); //Clears the buffers

    displayGrid();
    displayPoints();
    glFlush();
}


void keyPressed( unsigned char key, int xmouse, int ymouse)
{   
    switch (key){
        case 27:
        case 'Q':
        case 'q':
            exit(0);
            break;
        default:
            break;
    }
}

 
void init( int width, int height )
{
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize( width, height );
    glutInitWindowPosition( 50, 50 );
    glutCreateWindow( "SampleView" );

    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    //glEnable( GL_POINT_SMOOTH );
    glDisable( GL_DEPTH_TEST );
}


void readPoints( )
{
    std::copy( std::istream_iterator<float>( std::cin ),
               std::istream_iterator<float>(),
               std::inserter( samples, samples.begin() ) );
}

int main( int argc, char** argv ) {

    int width = 1024, height = 1024;
    
    grid_divisions = atoi( argv[1] );

    readPoints();

    glutInit( &argc, argv );
    init( width, height );
    glutKeyboardFunc( keyPressed );
    glutDisplayFunc( display );
    glutMainLoop(); //Process events etc. Also keeps the window open.
    return 0; //End the program
}
