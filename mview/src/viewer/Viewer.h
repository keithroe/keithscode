
/*****************************************************************************\
 *                                                                           *
 *  filename : Viewer.h                                                      *
 *  author   : R. Keith Morley                                               *
 *  last mod : 1/19/04                                                       *
 *                                                                           *
 *  General purpose mesh viewer meant to be run in a seperate thread.        *
 *                                                                           *
\*****************************************************************************/

#ifndef _VIEWER_H_
#define _VIEWER_H_ 1


#include <math/Vector3.h>
#include <mesh/BBox.h>
#include <viewer/Camera.h>
#include <viewer/Timer.h>
#include <vector>

class Mesh;


class Viewer
{
public:
    enum ShadeMode { FLAT, SMOOTH, LINE, HIDDENLINE };
    typedef std::vector<Mesh*> MeshList;

    // singleton accessor
    static Viewer& instance() 
    {
        if ( !_instance) 
            _instance = new Viewer();
        return *_instance;
    }

   
    void addMesh( Mesh* mesh )                 { _meshes.push_back(mesh); } 
    void view( int argc, char** argv );

    Camera&   camera()                         { return _camera; }
    int       nx()                             { return _nx; }
    int       ny()                             { return _ny; }
    BBox      bbox()                           { return _bbox; }
    ShadeMode shadeMode()                      { return _shade_mode; }
    Timer&    timer()                          { return _timer; }

    void setResolution( int nx, int ny)        { _nx = nx; _ny = ny; }
    void setShadeMode( int sm)                 { _shade_mode = (ShadeMode) sm; }
    void setButton( int button )   { _button = (Camera::MouseButton)button; }               
   
    void cycleShadeMode();                 

private:

    // this is a singleton class 
    Viewer();

    // glut functions
    static void display();
    static void changeShadeMode(int mode);
    static void idle();
    static void init();
    static void keyboard(unsigned char key, int x, int y);
    static void mouse(int button, int state, int x, int y);
    static void motion(int x, int y);
    static void reshape(int x, int y);
   
    static void printString( const char* s, int x, int y, 
            void* font = GLUT_BITMAP_8_BY_13 );
    void invertMouse( int x, int& y ) { y = _ny - y - 1; }


    int                 _nx;
    int                 _ny;

    ShadeMode           _shade_mode;
    int                 _main_window;
    BBox                _bbox;

    MeshList            _meshes;
    Camera              _camera;
    Camera::MouseButton _button;
    static Viewer*      _instance; 
    Timer               _timer;
};
  

#endif // _VIEWER_H_ 
