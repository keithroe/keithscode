       


#include <viewer/Arcball.h>
#include <GLUT/glut.h>


Arcball::Arcball()
{
    _location    = Vector3(256, 256, 0);
    _radius      = 256;
    _mouse       = Vector3(0.0, 0.0, 0.0);
    _mouse_down  = Vector3(0.0, 0.0, 0.0);
    _dragging    = false;
    _q_down      = Quaternion( 1.0, 0.0, 0.0, 0.0 );
    _q_now       = Quaternion( 1.0, 0.0, 0.0, 0.0 );
}



Arcball::Arcball(const Vector3 &location, float radius)
{
    _location    = location;
    _radius      = radius;
    _mouse       = Vector3(0.0, 0.0, 0.0);
    _mouse_down  = Vector3(0.0, 0.0, 0.0);
    _dragging    = false;
    _q_down      = Quaternion( 1.0, 0.0, 0.0, 0.0 );
    _q_now       = Quaternion( 1.0, 0.0, 0.0, 0.0 );
}



Vector3 Arcball::toSphere(const Vector3 &v)
{
    Vector3 sphere( (v.x() - _location.x()) / _radius,
                    (v.y() - _location.y()) / _radius,
                    0.0 );
    float magnitude = sphere.length();

    if (magnitude > 1.0)
        sphere /= sqrt(magnitude);
    else
        sphere.setZ( sqrt(1.0 - magnitude) );

    return sphere;
}



void Arcball::mouseDown( double x, double y )
{
    _mouse      = Vector3( x, y, 0.0 );
    _dragging   = true;
    _mouse_down = _mouse;
}



void Arcball::mouseUp( double x, double y )
{
    motion( x, y );

    _dragging    = false;
    _q_down = _q_now;
}



void Arcball::motion( double x, double y )
{
    _mouse = Vector3( x, y, 0.0 );
    if (_dragging)
    {
        Vector3    from = toSphere(_mouse_down);
        Vector3    to   = toSphere(_mouse);
        Quaternion q_drag(from, to);

        _q_now = q_drag * _q_down;
        _q_now.normalize();
    }
}



Quaternion Arcball::rotation()const
{
    return _q_now;
}


