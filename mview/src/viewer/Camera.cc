

#include "Camera.h"


Camera::Camera() :
    _arcball( Vector3( _nx / 2.0, _ny / 2.0, 0.0), _nx / 2.0 ),
    _fov(60.0), 
    _near(5.0), 
    _far(15.0), 
    _nx(512.0),
    _ny(512.0),
    _scene_radius(10.0),
    _scene_mid(0.0, 0.0, 0.0),
    _up( 0.0, 1.0, 0.0 ),
    _lookat( 0.0, 0.0, 0.0 ), 
    _eye( 0.0, 0.0, 10.0 ), 
    _scale( 1.0, 1.0, 1.0 ),
    _trans( 0.0, 0.0, 0.0 ),
    _mouse_x(0),
    _mouse_y(0),
    _projection( PERSPECTIVE )
{
}


Camera::~Camera()
{
}


void Camera::setSceneBBox( const BBox& bbox )
{
    _scene_mid    = bbox.mid();
    _scene_radius = (bbox.max() - bbox.mid()).length();
    _lookat       = Vector3( 0.0, 0.0, 0.0 );
    _eye          = _lookat + Vector3( 0.0, 0.0, 4.0 * _scene_radius ); 
}


void Camera::updateNearFar() 
{
    double z_radius = _scene_radius * _scale.z();
    double z_dist   = _eye.z() - _scene_mid.z() + _trans.z();
    _near           = z_dist - z_radius + _trans.z();
    _far            = z_dist + z_radius + _trans.z();

    _near = _near < 0.001 ? 0.001 : _near;
}


void Camera::setResolution( int nx, int ny)         
{ 
    _nx = nx; 
    _ny = ny; 
    _arcball.reset( Vector3( _nx/2.0, _ny/2.0, 0.0), _nx/2.0 );
}


void Camera::motion( MouseButton button, int x, int y )
{
    const double dx = x - _mouse_x;
    const double dy = y - _mouse_y;
   
    if ( button == LEFT ) {

        // rotate
        _arcball.motion( x, y );

    } else if ( button == RIGHT ) {

        // zoom
        double scale = _scale.x();
        scale *= exp(-dy * 0.01);
        _scale = Vector3( scale, scale, scale );

    } else { // button == MIDDLE 

        // translate
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);

        _trans = Vector3( _trans.x() - dx*2.0*_scene_radius/(double)viewport[2],
                          _trans.y() - dy*2.0*_scene_radius/(double)viewport[3],
                          0.0);

    }
    
    _mouse_x = x;
    _mouse_y = y;
}


void Camera::mouseUp( MouseButton button,int x, int y )
{
    _mouse_x = x;
    _mouse_y = y;
   
    if ( button == LEFT ) {
        // rotate
        _arcball.mouseUp( x, y );
    } else if ( button == RIGHT ) {
        // zoom
    } else { // button == MIDDLE 
        // translate
    }
}


void Camera::mouseDown( MouseButton button, int x, int y )
{
    _mouse_x = x;
    _mouse_y = y;
    
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );


    if ( button == LEFT ) {
        // rotate
        _arcball.mouseDown( x, y );
    } else if ( button == RIGHT ) {
        // zoom
    } else { // button == MIDDLE 
        // translate
    }
}


void Camera::applyProjection()
{
    updateNearFar();
    gluPerspective( _fov, _nx / _ny, _near, _far );
}


void Camera::applyView()
{
    gluLookAt( _eye.x(),    _eye.y(),    _eye.z(), 
               _lookat.x(), _lookat.y(), _lookat.z(), 
               _up.x(),     _up.y(),     _up.z()      );

    /*
    glTranslated( -_trans.x(), -_trans.y(), -_trans.z() );
    
    double angle, x, y, z;
    _arcball.rotation( angle, x, y, z );
    glRotatef( angle, x, y, z );
   
    glScaled( _scale.x(), _scale.y(), _scale.z() );
    */
}


void Camera::applyModel()
{
    glTranslated( -_trans.x(), -_trans.y(), -_trans.z() );
    
    double angle, x, y, z;
    _arcball.rotation( angle, x, y, z );
    glRotatef( angle, x, y, z );
  
    glScaled( _scale.x(), _scale.y(), _scale.z() );
    
    glTranslated( -_scene_mid.x(), -_scene_mid.y(), -_scene_mid.z() );
}




