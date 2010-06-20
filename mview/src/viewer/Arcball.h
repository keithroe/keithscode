

        

#ifndef _ARCBALL_H_
#define _ARCBALL_H_

#include <math/Vector3.h>
#include <math/Quaternion.h>

class Arcball
{
public:
    Arcball(const Vector3& location, float radius);
    
    void reset( const Vector3& location, double radius )
    { _location = location;  _radius = radius; } 

    void mouseDown( double x, double y );
    void mouseUp( double x, double y );
    void motion( double x, double y );
    Quaternion rotation()const;
    void rotation( double& angle, double& x, double& y, double& z)const
    { _q_now.rotation( angle, x, y, z ); }

private:
    Arcball();
    Vector3     toSphere(const Vector3 &v);

    Vector3     _location;
    float       _radius;

    Vector3     _mouse;
    Vector3     _mouse_down;

    bool        _dragging;

    Quaternion  _q_down; // _orientation;
    Quaternion  _q_now;  // _current;

};


#endif // _ARCBALL_H_

