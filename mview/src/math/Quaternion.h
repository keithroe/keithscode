
/*****************************************************************************\ 
 *  
 *  filename: Quaternion.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
 \*****************************************************************************/


#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include <math/Vector3.h>
#include <math/MathUtility.h>
#include <cassert>
#include <iostream>

class Quaternion {
public:
    Quaternion()
    { q[0] = q[1] = q[2] = q[3] = 0.0; }

    Quaternion( double w, double x, double y, double z ) 
    { q[0] = w; q[1] = x; q[2] = y; q[3] = z; }

    Quaternion( const Vector3& from, const Vector3& to );

    Quaternion( const Quaternion& a ) 
    { q[0] = a[0];  q[1] = a[1];  q[2] = a[2];  q[3] = a[3]; }

    Quaternion ( double angle, const Vector3&  axis );

    // getters and setters
    void setW(double _w)       { q[0] = _w; }
    void setX(double _x)       { q[1] = _x; }
    void setY(double _y)       { q[2] = _y; }
    void setZ(double _z)       { q[3] = _z; }
    double w() const           { return q[0]; }
    double x() const           { return q[1]; }
    double y() const           { return q[2]; }
    double z() const           { return q[3]; }


    Quaternion& operator-=(const Quaternion& r)
    { q[0] -= r[0]; q[1] -= r[1]; q[2] -= r[2]; q[3] -= r[3]; return *this; }

    Quaternion& operator+=(const Quaternion& r)
    { q[0] += r[0]; q[1] += r[1]; q[2] += r[2]; q[3] += r[3]; return *this; }

    Quaternion& operator*=(const Quaternion& r);

    Quaternion& operator/=(const double a);
    
    Quaternion conjugate()
    { return Quaternion( q[0], -q[1], -q[2], -q[3] ); }

    void rotate( Vector3& v ) const;
    void rotation( double& angle, Vector3& axis ) const;
    void rotation( double& angle, double& x, double& y, double& z ) const;

    double& operator[](int i)      { assert( i>=0 && i< 4); return q[i]; }
    double operator[](int i)const  { assert( i>=0 && i< 4); return q[i]; }

    // l2 norm
    double norm() const
    { return sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]); }

    double  normalize();

private:
        double q[4];
};


inline Quaternion::Quaternion( const Vector3& from, const Vector3& to ) 
{ 
    Vector3 c = cross( from, to );
    q[0] = dot(from, to); 
    q[1] = c[0];
    q[2] = c[1];
    q[3] = c[2];
} 


inline Quaternion::Quaternion( double angle, const Vector3&  axis )
{ 
    Vector3 naxis(axis);
    double n        = naxis.normalize(); 
    assert( n > MView::EPSILON );
    angle          *= MView::DEG2RAD;
    double inverse  = 1.0/n;
    double s        = sin(angle/2.0);
    q[0] = naxis[0]*s*inverse;   
    q[1] = naxis[1]*s*inverse; 
    q[2] = naxis[2]*s*inverse; 
    q[3] = cos(angle/2.0);
}


inline Quaternion& Quaternion::operator*=(const Quaternion& r)
{    
  
    double w = q[0]*r[0] - q[1]*r[1] - q[2]*r[2] - q[3]*r[3]; 
    double x = q[0]*r[1] + q[1]*r[0] + q[2]*r[3] - q[3]*r[2];
    double y = q[0]*r[2] + q[2]*r[0] + q[3]*r[1] - q[1]*r[3];
    double z = q[0]*r[3] + q[3]*r[0] + q[1]*r[2] - q[2]*r[1];
    
    q[0] = w;
    q[1] = x;
    q[2] = y;
    q[3] = z;
    return *this; 
}


inline Quaternion& Quaternion::operator/=(const double a)
{ 
    double inverse = 1.0/a;
    q[0] *= inverse; 
    q[1] *= inverse; 
    q[2] *= inverse; 
    q[3] *= inverse; 
    return *this; 
}


inline void Quaternion::rotate( Vector3& v ) const
{
    
}


inline void Quaternion::rotation( double& angle, Vector3& axis ) const
{ 
    Quaternion n = *this; 
    n.normalize();
    axis.setX(n[1]); 
    axis.setY(n[2]); 
    axis.setZ(n[3]); 
    angle = MView::RAD2DEG *2.0 * acos(n[0]);
}

    
inline void Quaternion::rotation( double& angle, double& x, double& y, 
        double& z ) const
{ 
    Quaternion n = *this; 
    n.normalize();
    x = n[1]; 
    y = n[2]; 
    z = n[3]; 
    angle = MView::RAD2DEG *2.0 * acos(n[0]);
}


inline double Quaternion::normalize()
{ 
    double n = norm();
    double inverse = 1.0/n; 
    q[0] *= inverse; 
    q[1] *= inverse; 
    q[2] *= inverse; 
    q[3] *= inverse; 
    return n; 
}


inline Quaternion operator*(const double a, const Quaternion &r)
{ return Quaternion(a*r[0], a*r[1], a*r[2], a*r[3]); }


inline Quaternion operator*(const Quaternion &r, const double a)
{ return Quaternion(a*r[0], a*r[1], a*r[2], a*r[3]); }


inline Quaternion operator/(const Quaternion &r, const double a)
{ 
    double inverse = 1.0/a;
    return Quaternion( r[0]*inverse, r[1]*inverse, r[2]*inverse, r[3]*inverse); 
}


inline Quaternion operator/(const double a, const Quaternion &r)
{ 
    double inverse = 1.0/a;
    return Quaternion( r[0]*inverse, r[1]*inverse, r[2]*inverse, r[3]*inverse); 
}


inline Quaternion operator-(const Quaternion& l, const Quaternion& r)
{ return Quaternion(l[0]-r[0], l[1]-r[1], l[2]-r[2], l[3]-r[3]); }


inline bool operator==(const Quaternion& l, const Quaternion& r)
{ return ( l[0] == r[0] && l[1] == r[1] && l[2] == r[2] && l[3] == r[3] ); }


inline bool operator!=(const Quaternion& l, const Quaternion& r)
{ return !(l == r); }


inline Quaternion operator+(const Quaternion& l, const Quaternion& r)
{ return Quaternion(l[0]+r[0], l[1]+r[1], l[2]+r[2], l[3]+r[3]); }


inline Quaternion operator*(const Quaternion& l, const Quaternion& r)
{ 
    double w = l[0]*r[0] - l[1]*r[1] - l[2]*r[2] - l[3]*r[3]; 
    double x = l[0]*r[1] + l[1]*r[0] + l[2]*r[3] - l[3]*r[2];
    double y = l[0]*r[2] + l[2]*r[0] + l[3]*r[1] - l[1]*r[3];
    double z = l[0]*r[3] + l[3]*r[0] + l[1]*r[2] - l[2]*r[1];
    return Quaternion( w, x, y, z );
}

inline double dot( const Quaternion& l, const Quaternion& r )
{
    return l.w()*r.w() + l.x()*r.x() + l.y()*r.y() + l.z()*r.z();
}

inline std::ostream& operator<<( std::ostream& out, const Quaternion& q )
{
    return out << "[ (" << q.x() << ", " << q.y() << ", " << q.z() << ") " 
               << q.w() << "]";
}

#endif // _QUATERNION_H_

