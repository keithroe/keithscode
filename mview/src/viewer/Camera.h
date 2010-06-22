
/*****************************************************************************\ 
 *  
 *  filename: Camera.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <viewer/Arcball.h>
#include <mesh/BBox.h>

#if defined(__APPLE__)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


class  Camera
{
public:

    enum ProjectionType { ORTHOGRAPHIC = 0, 
                          PERSPECTIVE  = 1 };
    enum MouseButton    { LEFT         = GLUT_LEFT_BUTTON, 
                          RIGHT        = GLUT_RIGHT_BUTTON, 
                          MIDDLE       = GLUT_MIDDLE_BUTTON };


    Camera();
    ~Camera();

    void setSceneBBox( const BBox& bbox );
    void setResolution( int nx, int ny);
    void setPerspective( double fov )           { _fov = fov; }
    void setProjection( ProjectionType type )   { _projection = type; }

    void motion( MouseButton button, int x, int y);
    void mouseUp( MouseButton button, int x, int y);
    void mouseDown( MouseButton button, int x, int y);

    void setOrientation( const Vector3& eye, 
                       const Vector3& lookat, 
                       const Vector3& up ) 
    { _eye = eye; _up = up; _lookat = lookat; }

    void applyProjection();
    void applyView();
    void applyModel();

private:
    void updateNearFar();

    Arcball _arcball;
    double  _fov;
    double  _near;
    double  _far;
    double  _nx;
    double  _ny;

    double  _scene_radius;
    Vector3 _scene_mid;

    Vector3 _up;
    Vector3 _lookat;
    Vector3 _eye;

    Vector3 _scale;
    Vector3 _trans;

    int     _mouse_x;
    int     _mouse_y;

    ProjectionType _projection;
};


#endif // _CAMERA_H_
