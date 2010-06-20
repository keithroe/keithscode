
#ifndef _ARCBALL_
#define _ARCBALL_

#include <math/Quaternion.h>


class Arcball {
public:
    
    Arcball(double xc = 0.0, double yc = 0.0, double r = 1.0, 
            Quaternion qt = Quaternion(1.0, 0.0, 0.0, 0.0) );

    void reset( double xc, double yc, double r );

    void mouseDown( double x, double y );

    void motion( double x, double y );

    void mouseUp( double x, double y );

    void rotation( double& angle, Vector3& axis )const;
    
    void rotation( double& angle, double &x, double &y, double &z )const;

private:

    Vector3 getv( double x, double y );

    int drag;
    double xc, yc, r;
    Vector3 ei, ef;
    Quaternion qt, q;
};


inline Arcball::Arcball(double uxc, double uyc, double ur, Quaternion uqt)
{
    xc = uxc;
    yc = uyc;
    r = ur;
    qt = uqt;
    q = Quaternion( 1.0, 0.0, 0.0, 0.0 );
}


inline void Arcball::reset(double uxc, double uyc, double ur)
{
    xc = uxc;
    yc = uyc;
    r = ur;
    q = Quaternion( 1.0, 0.0, 0.0, 0.0 );
}


inline void Arcball::mouseDown(double x, double y)
{
    // get mouse down position in ball
    ei = getv(x, y);
    // reset current rotation
    q = Quaternion( 1.0, 0.0, 0.0, 0.0 );
    // begin dragging
    drag = true;
}


inline void Arcball::mouseUp(double x, double y)
{
    // make sure we move to final position
    motion(x, y);
    // add current rotation into cumulative rotation
    qt = q * qt;
    // finish dragging
    drag = false;
    // reset current rotation
    q = Quaternion( 1.0, 0.0, 0.0, 0.0 );
}


inline void Arcball::motion(double x, double y)
{
    // if no dragging, ignore
    if (!drag) return;
    // get mouse position in ball
    ef = getv(x, y);
    // get rotation axis
    Vector3 v = cross(ei, ef);
    // get rotation angle
    double a = dot(ei, ef);
    // create rotation from mouse down position to current position
    q = Quaternion( a, v[0], v[1], v[2] );
}


inline void Arcball::rotation( double& angle, Vector3& axis ) const
{
    // add current rotation to cumulative rotation
    Quaternion qr = q * qt;
    // return total rotation
    return qr.rotation(angle, axis);
}


inline void Arcball::rotation( double& angle, double &x, double &y, 
        double &z ) const
{
    // add current rotation to cumulative rotation
    Quaternion qr = q * qt;
    // return total rotation
    qr.rotation( angle, x, y, z );
}


inline Vector3 Arcball::getv(double x, double y)
{
    x = (x - xc) / r;
    y = (y - yc) / r;
    double n = x*x + y*y;
    // if point is out of sphere area
    if (n >= 1.0) 
    {
        n = sqrt(n);
        return Vector3(x/n, y/n, 0.0);
    } 
    // if point is in sphere area
    else 
        return Vector3(x, y, sqrt(1-n));
}

#endif // _ARCBALL_
