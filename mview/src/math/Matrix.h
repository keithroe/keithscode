
/******************************************************************************\
 *                                                                            *
 *  filename   : Matrix.h                                                     *
 *  author     : R. Keith Morley                                              *
 *  last mod   : 2/28/02                                                      *
 *                                                                            *
 *  Matrix class for use in graphics applications                             *
 *                                                                            *
\******************************************************************************/

#ifndef _MATRIX_H_
#define _MATRIX_H_ 1

#include <math/Vector3.h>
#include <iostream>

class Matrix
{
public:
   Matrix();
   Matrix(const Matrix & orig);

   void invert();
   void transpose();
   Matrix getInverse()const;
   Matrix getTranspose()const;
   bool isIdentity()const;

   Matrix& operator=  (const Matrix& right_op);
   Matrix& operator+= (const Matrix& right_op);
   Matrix& operator-= (const Matrix& right_op);
   Matrix& operator*= (const Matrix& right_op);
   Matrix& operator*= (double right_op);
   double* operator[](int a) { return x[a]; }

   friend Matrix operator- (const Matrix& left_op, const Matrix& right_op);
   friend Matrix operator+ (const Matrix& left_op, const Matrix& right_op);
   friend Matrix operator* (const Matrix& left_op, const Matrix& right_op);
   // matrix mult. performed left to right
   friend Vector3 operator* (const Matrix& left_op, const Vector3& right_op);
   // transform vector by a matrix
   friend Matrix operator* (const Matrix & left_op, double right_op);

   friend Vector3 transformLoc(const Matrix& left_op, const Vector3& right_op);
   friend Vector3 transformVec(const Matrix& left_op, const Vector3& right_op);

   friend Matrix zeroMatrix();
   friend Matrix identityMatrix();
   friend Matrix translate(double _x, double _y, double _z);
   friend Matrix scale(double _x, double _y, double _z);
   friend Matrix rotate(const Vector3 & axis, double angle);
   friend Matrix rotateX(double angle);    //
   friend Matrix rotateY(double angle);    // More efficient than arbitrary axis
   friend Matrix rotateZ(double angle);    //
   friend Matrix lookAt(const Vector3& eye, const Vector3 & at,
           const Vector3& up);
   friend std::ostream & operator<< (std::ostream& out, const Matrix& right_op);

   double determinant();
   double x[4][4];
};

// 3x3 matrix determinant -- helper function
inline double det3 (double a, double b, double c,
                   double d, double e, double f,
                   double g, double h, double i)
{ return a*e*i + d*h*c + g*b*f - g*e*c - d*b*i - a*h*f; }

Matrix operator- (const Matrix& left_op, const Matrix& right_op);
Matrix operator+ (const Matrix& left_op, const Matrix& right_op);
Matrix operator* (const Matrix& left_op, const Matrix& right_op);
// matrix mult. performed left to right
Vector3 operator* (const Matrix& left_op, const Vector3& right_op);
// transform vector by a matrix
Matrix operator* (const Matrix & left_op, double right_op);

Vector3 transformLoc(const Matrix& left_op, const Vector3& right_op);
Vector3 transformVec(const Matrix& left_op, const Vector3& right_op);

Matrix zeroMatrix();
Matrix identityMatrix();
Matrix translate(double _x, double _y, double _z);
Matrix scale(double _x, double _y, double _z);
Matrix rotate(const Vector3 & axis, double angle);
Matrix rotateX(double angle);    //
Matrix rotateY(double angle);    // More efficient than arbitrary axis
Matrix rotateZ(double angle);    //
Matrix lookAt(const Vector3& eye, const Vector3 & at,
              const Vector3& up);
std::ostream & operator<< (std::ostream& out, const Matrix& right_op);


#endif   // _MATRIX_H_
