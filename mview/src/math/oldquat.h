//=======================================================================
// Quaternion class implementation.
// 19/7/99, Diego Nehab
//=======================================================================

#ifndef _QUATERNION_
#define _QUATERNION_

#include "scalar.h"
#include "vector.h"

//=======================================================================
// Quaternion Class
//=======================================================================
class Quaternion {
public:
    //---------------------------------------------------------------------
    // Constructor
    // Input
    //     x, y, z, w: quaternion coordinates. all values default to zero.
    //---------------------------------------------------------------------
    Quaternion (Scalar x = 0.0, Scalar y = 0.0, Scalar z = 0.0, Scalar w = 0.0) 
    { q[0] = x; q[1] = y; q[2] = z; q[3] = w; }

    //---------------------------------------------------------------------
    // Constructor
    // Input
    //     v, q: rotation vector and agle;
    //---------------------------------------------------------------------
    Quaternion (Scalar a, Vector v)
    { a *= SCALAR_TORADIAN;
      Scalar n = v.Norm(); if (isZero(n)) n = 1;
      Scalar inv_n = 1.0/n;
      Scalar s = sin(a/2);
      q[0] = v[0]*s*inv_n; q[1] = v[1]*s*inv_n; 
      q[2] = v[2]*s*inv_n; q[3] = cos(a/2);
    }

    //---------------------------------------------------------------------
    // Print quaternion
    //---------------------------------------------------------------------
    void Print(void) const
    { printf("(%f, %f, %f, %f)", q[0], q[1], q[2], q[3]); };

    //---------------------------------------------------------------------
    // Redefine quaternion
    // Input
    //     ux, uy, uz: coordinates.
    //---------------------------------------------------------------------
    void X(Scalar x) { q[0] = x; }
    void Y(Scalar y) { q[1] = y; }
    void Z(Scalar z) { q[2] = z; }
    void W(Scalar w) { q[3] = w; }

    //---------------------------------------------------------------------
    // Access quaternion coordinates
    // Returns
    //     corresponding coordinate
    //---------------------------------------------------------------------
    Scalar X(void) const { return q[0]; };
    Scalar Y(void) const { return q[1]; };
    Scalar Z(void) const { return q[2]; };
    Scalar W(void) const { return q[3]; };

    //---------------------------------------------------------------------
    // Unary minus operator
    // Returns
    //     -Quaternion
    //---------------------------------------------------------------------
    Quaternion operator-() const 
    { return Quaternion(-q[0], -q[1], -q[2], -q[3]); };

    //---------------------------------------------------------------------
    // Quaternion subtraction
    // Returns
    //     Quaternion - Quaternion
    //---------------------------------------------------------------------
    Quaternion operator-(const Quaternion& r) const
    { return Quaternion(q[0]-r[0], q[1]-r[1], q[2]-r[2], q[3]-r[3]); };

    //---------------------------------------------------------------------
    // Quaternion subtraction
    // Returns
    //     Quaternion -= Quaternion
    //---------------------------------------------------------------------
    Quaternion& operator-=(const Quaternion& r)
    { q[0] -= r[0]; q[1] -= r[1]; q[2] -= r[2]; q[3] -= r[3]; return *this; };

    //---------------------------------------------------------------------
    // Quaternion addition
    // Returns
    //     Quaternion + Quaternion
    //---------------------------------------------------------------------
    Quaternion operator+(const Quaternion& r) const
    { return Quaternion(q[0]+r[0], q[1]+r[1], q[2]+r[2], q[3]+r[3]); };

    //---------------------------------------------------------------------
    // Quaternion addition
    // Returns
    //     Quaternion += Quaternion
    //---------------------------------------------------------------------
    Quaternion& operator+=(const Quaternion& r)
    { q[0] += r[0]; q[1] += r[1]; q[2] += r[2]; q[3] += r[3]; return *this; };

    //---------------------------------------------------------------------
    // Quaternion product
    // Returns
    //     Quaternion * Quaternion
    //---------------------------------------------------------------------
    Quaternion operator*(const Quaternion& r) const
    { return Quaternion(
            q[3]*r[0] + q[0]*r[3] + q[1]*r[2] - q[2]*r[1],
            q[3]*r[1] + q[1]*r[3] + q[2]*r[0] - q[0]*r[2],
            q[3]*r[2] + q[2]*r[3] + q[0]*r[1] - q[1]*r[0],
            q[3]*r[3] - q[0]*r[0] - q[1]*r[1] - q[2]*r[2]); };

    //---------------------------------------------------------------------
    // Quaternion product
    // Returns
    //     Quaternion *= Quaternion
    //---------------------------------------------------------------------
    Quaternion& operator*=(const Quaternion& r)
    { Scalar x = q[3]*r[0] + q[0]*r[3] + q[1]*r[2] - q[2]*r[1];
      Scalar y = q[3]*r[1] + q[1]*r[3] + q[2]*r[0] - q[0]*r[2];
      Scalar z = q[3]*r[2] + q[2]*r[3] + q[0]*r[1] - q[1]*r[0];
      Scalar w = q[3]*r[3] - q[0]*r[0] - q[1]*r[1] - q[2]*r[2]; 
      q[0] = x; q[1] = y; q[2] = z; q[3] = w; return *this; };

    //---------------------------------------------------------------------
    // Division by scalar
    // Returns
    //     Quaternion / Scalar
    //---------------------------------------------------------------------
    Quaternion operator/(const Scalar a) const
    { Scalar inv_a = 1.0/a;
        return Quaternion(q[0]*inv_a, q[1]*inv_a, q[2]*inv_a, q[3]*inv_a); };

    //---------------------------------------------------------------------
    // Division by scalar
    // Returns
    //     Quaternion /= Scalar
    //---------------------------------------------------------------------
    Quaternion& operator/=(const Scalar a)
    { Scalar inv_a = 1.0/a;
        q[0] *= inv_a; q[1] *= inv_a; q[2] *= inv_a; 
        q[3] *= inv_a; return *this; };

    //---------------------------------------------------------------------
    // Equallity test
    // Returns
    //     true if quaternions are equal
    //---------------------------------------------------------------------
    bool operator==(const Quaternion& r) const
    { return (isEqual(q[0], r[0]) && isEqual(q[1], r[1]) && 
            isEqual(q[2], r[2]) && isEqual(q[3], q[3])); };

    //---------------------------------------------------------------------
    // Inequallity test
    // Returns
    //     true if quaternions are different
    //---------------------------------------------------------------------
    bool operator!=(const Quaternion& r)const
    { return (!isEqual(q[0],r[0]) || !isEqual(q[1],r[1]) || 
            !isEqual(q[2],r[2]) || !isEqual(q[3], q[3])); };


    //---------------------------------------------------------------------
    // Gets rotation corresponding to quaternion
    // Returns
    //     rotation axis, rotation angle
    //---------------------------------------------------------------------
    Scalar Rotation(Vector &v) const
    { Quaternion n = *this; n.Normalize();
      v.X(n[0]); v.Y(n[1]); v.Z(n[2]); 
      return SCALAR_TODEGREE*2*acos(n[3]);
    }

    //---------------------------------------------------------------------
    // Gets rotation corresponding to quaternion
    // Returns
    //     rotation axis, rotation angle
    //---------------------------------------------------------------------
    Scalar Rotation(Scalar &x, Scalar &y, Scalar &z) const
    { Quaternion n = *this; n.Normalize();
      x = n[0]; y = n[1]; z = n[2]; 
      return SCALAR_TODEGREE*2*acos(n[3]);
    }

    //---------------------------------------------------------------------
    // Indexing operator
    // Returns
    //     corresponding component
    //---------------------------------------------------------------------
    Scalar operator[](int c) const
    { return q[c]; };

    //---------------------------------------------------------------------
    // Determine l2 norm
    // Returns
    //     Quaternion's l2 norm
    //---------------------------------------------------------------------
    Scalar Norm(void) const
    { return sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]); };

    //---------------------------------------------------------------------
    // Normalize quaternion
    // Returns
    //     changed quaternion
    //---------------------------------------------------------------------
    Quaternion& Normalize(void)
    { Scalar inv_n = 1.0/Norm(); 
        q[0] *= inv_n; q[1] *= inv_n; q[2] *= inv_n; q[3] *= inv_n; 
        return *this; };
 
private:
    // quaternion coordinates
    Scalar q[4];
};

//=======================================================================
// Auxiliar functions
//=======================================================================
inline Quaternion Unitary(const Quaternion &r)
{
    Quaternion n = r;
    return n.Normalize();
}

inline Quaternion Unitary(const Quaternion *r)
{
    Quaternion n = *r;
    return n.Normalize();
}

inline Quaternion operator*(const Scalar a, const Quaternion &r)
{
    return Quaternion(a*r[0], a*r[1], a*r[2]);
}

inline Quaternion operator*(const Quaternion &r, const Scalar a)
{
    return a*r;
}

#endif // _QUATERNION_

