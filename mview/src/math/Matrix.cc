
#include <math/Matrix.h>
#include <iomanip>
#include <cmath>
#include <cstdio>

 Matrix::Matrix()
{
   x[0][0] = 1.0;
   x[0][1] = 0.0;
   x[0][2] = 0.0;
   x[0][3] = 0.0;
   x[1][0] = 0.0;
   x[1][1] = 1.0;
   x[1][2] = 0.0;
   x[1][3] = 0.0;
   x[2][0] = 0.0;
   x[2][1] = 0.0;
   x[2][2] = 1.0;
   x[2][3] = 0.0;
   x[3][0] = 0.0;
   x[3][1] = 0.0;
   x[3][2] = 0.0;
   x[3][3] = 1.0;
}

Matrix::Matrix (const Matrix & right_op)
{
   x[0][0] = right_op.x[0][0];
   x[0][1] = right_op.x[0][1];
   x[0][2] = right_op.x[0][2];
   x[0][3] = right_op.x[0][3];
   x[1][0] = right_op.x[1][0];
   x[1][1] = right_op.x[1][1];
   x[1][2] = right_op.x[1][2];
   x[1][3] = right_op.x[1][3];
   x[2][0] = right_op.x[2][0];
   x[2][1] = right_op.x[2][1];
   x[2][2] = right_op.x[2][2];
   x[2][3] = right_op.x[2][3];
   x[3][0] = right_op.x[3][0];
   x[3][1] = right_op.x[3][1];
   x[3][2] = right_op.x[3][2];
   x[3][3] = right_op.x[3][3];
}

Matrix& Matrix::operator= (const Matrix & right_op)
{
   x[0][0] = right_op.x[0][0];
   x[0][1] = right_op.x[0][1];
   x[0][2] = right_op.x[0][2];
   x[0][3] = right_op.x[0][3];
   x[1][0] = right_op.x[1][0];
   x[1][1] = right_op.x[1][1];
   x[1][2] = right_op.x[1][2];
   x[1][3] = right_op.x[1][3];
   x[2][0] = right_op.x[2][0];
   x[2][1] = right_op.x[2][1];
   x[2][2] = right_op.x[2][2];
   x[2][3] = right_op.x[2][3];
   x[3][0] = right_op.x[3][0];
   x[3][1] = right_op.x[3][1];
   x[3][2] = right_op.x[3][2];
   x[3][3] = right_op.x[3][3];
   return *this;
}


Matrix& Matrix::operator+= (const Matrix & right_op)
{
   x[0][0] += right_op.x[0][0];
   x[0][1] += right_op.x[0][1];
   x[0][2] += right_op.x[0][2];
   x[0][3] += right_op.x[0][3];
   x[1][0] += right_op.x[1][0];
   x[1][1] += right_op.x[1][1];
   x[1][2] += right_op.x[1][2];
   x[1][3] += right_op.x[1][3];
   x[2][0] += right_op.x[2][0];
   x[2][1] += right_op.x[2][1];
   x[2][2] += right_op.x[2][2];
   x[2][3] += right_op.x[2][3];
   x[3][0] += right_op.x[3][0];
   x[3][1] += right_op.x[3][1];
   x[3][2] += right_op.x[3][2];
   x[3][3] += right_op.x[3][3];
   return *this;
}

Matrix& Matrix::operator-= (const Matrix & right_op)
{
   x[0][0] -= right_op.x[0][0];
   x[0][1] -= right_op.x[0][1];
   x[0][2] -= right_op.x[0][2];
   x[0][3] -= right_op.x[0][3];
   x[1][0] -= right_op.x[1][0];
   x[1][1] -= right_op.x[1][1];
   x[1][2] -= right_op.x[1][2];
   x[1][3] -= right_op.x[1][3];
   x[2][0] -= right_op.x[2][0];
   x[2][1] -= right_op.x[2][1];
   x[2][2] -= right_op.x[2][2];
   x[2][3] -= right_op.x[2][3];
   x[3][0] -= right_op.x[3][0];
   x[3][1] -= right_op.x[3][1];
   x[3][2] -= right_op.x[3][2];
   x[3][3] -= right_op.x[3][3];
   return *this;
}

Matrix& Matrix::operator*= (double right_op)
{
   x[0][0] *= right_op;
   x[0][1] *= right_op;
   x[0][2] *= right_op;
   x[0][3] *= right_op;
   x[1][0] *= right_op;
   x[1][1] *= right_op;
   x[1][2] *= right_op;
   x[1][3] *= right_op;
   x[2][0] *= right_op;
   x[2][1] *= right_op;
   x[2][2] *= right_op;
   x[2][3] *= right_op;
   x[3][0] *= right_op;
   x[3][1] *= right_op;
   x[3][2] *= right_op;
   x[3][3] *= right_op;
   return *this;
}

Matrix& Matrix::operator*= (const Matrix & right_op)
{
   Matrix ret = *this;

   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
      {
         double sum = 0;
         for (int k = 0; k < 4; k++)
            sum += ret.x[i][k] * right_op.x[k][j];
         x[i][j] = sum;
      }
   return *this;
}

// based on Gordon Kindlemann's code
void Matrix::invert()
{
   double det = determinant();
   Matrix inverse;
   inverse.x[0][0]  = det3(x[1][1], x[1][2], x[1][3],
                           x[2][1], x[2][2], x[2][3],
                           x[3][1], x[3][2], x[3][3]) / det;
   inverse.x[0][1] = -det3(x[0][1], x[0][2], x[0][3],
                           x[2][1], x[2][2], x[2][3],
                           x[3][1], x[3][2], x[3][3]) / det;
   inverse.x[0][2]  = det3(x[0][1], x[0][2], x[0][3],
                           x[1][1], x[1][2], x[1][3],
                           x[3][1], x[3][2], x[3][3]) / det;
   inverse.x[0][3] = -det3(x[0][1], x[0][2], x[0][3],
                           x[1][1], x[1][2], x[1][3],
                           x[2][1], x[2][2], x[2][3]) / det;

   inverse.x[1][0] = -det3(x[1][0], x[1][2], x[1][3],
                           x[2][0], x[2][2], x[2][3],
                           x[3][0], x[3][2], x[3][3]) / det;
   inverse.x[1][1]  = det3(x[0][0], x[0][2], x[0][3],
                           x[2][0], x[2][2], x[2][3],
                           x[3][0], x[3][2], x[3][3]) / det;
   inverse.x[1][2] = -det3(x[0][0], x[0][2], x[0][3],
                           x[1][0], x[1][2], x[1][3],
                           x[3][0], x[3][2], x[3][3]) / det;
   inverse.x[1][3]  = det3(x[0][0], x[0][2], x[0][3],
                           x[1][0], x[1][2], x[1][3],
                           x[2][0], x[2][2], x[2][3]) / det;

   inverse.x[2][0]  = det3(x[1][0], x[1][1], x[1][3],
                           x[2][0], x[2][1], x[2][3],
                           x[3][0], x[3][1], x[3][3]) / det;
   inverse.x[2][1] = -det3(x[0][0], x[0][1], x[0][3],
                           x[2][0], x[2][1], x[2][3],
                           x[3][0], x[3][1], x[3][3]) / det;
   inverse.x[2][2]  = det3(x[0][0], x[0][1], x[0][3],
                           x[1][0], x[1][1], x[1][3],
                           x[3][0], x[3][1], x[3][3]) / det;
   inverse.x[2][3] = -det3(x[0][0], x[0][1], x[0][3],
                           x[1][0], x[1][1], x[1][3],
                           x[2][0], x[2][1], x[2][3]) / det;

   inverse.x[3][0] = -det3(x[1][0], x[1][1], x[1][2],
                           x[2][0], x[2][1], x[2][2],
                           x[3][0], x[3][1], x[3][2]) / det;
   inverse.x[3][1] =  det3(x[0][0], x[0][1], x[0][2],
                           x[2][0], x[2][1], x[2][2],
                           x[3][0], x[3][1], x[3][2]) / det;
   inverse.x[3][2] = -det3(x[0][0], x[0][1], x[0][2],
                           x[1][0], x[1][1], x[1][2],
                           x[3][0], x[3][1], x[3][2]) / det;
   inverse.x[3][3] =  det3(x[0][0], x[0][1], x[0][2],
                           x[1][0], x[1][1], x[1][2],
                           x[2][0], x[2][1], x[2][2]) / det;

  *this = inverse;

}

Matrix Matrix::getInverse()const
{
   Matrix ret = *this;
   ret.invert();
   return ret;
}

bool Matrix::isIdentity()const
{
    return ( 0.9999 < x[0][0]  && x[0][0] < 1.0001 && 
             0.9999 < x[1][1]  && x[1][1] < 1.0001 && 
             0.9999 < x[2][2]  && x[2][2] < 1.0001 && 
             0.9999 < x[3][3]  && x[3][3] < 1.0001 && 
            -0.0001 < x[0][1]  && x[0][1] < 0.0001 && 
            -0.0001 < x[0][2]  && x[0][2] < 0.0001 && 
            -0.0001 < x[0][3]  && x[0][3] < 0.0001 && 
            
            -0.0001 < x[1][0]  && x[1][0] < 0.0001 && 
            -0.0001 < x[1][2]  && x[1][2] < 0.0001 && 
            -0.0001 < x[1][3]  && x[1][3] < 0.0001 && 
            
            -0.0001 < x[2][0]  && x[2][0] < 0.0001 && 
            -0.0001 < x[2][1]  && x[2][1] < 0.0001 && 
            -0.0001 < x[2][3]  && x[2][3] < 0.0001 && 
            
            -0.0001 < x[3][0]  && x[3][0] < 0.0001 && 
            -0.0001 < x[3][1]  && x[3][1] < 0.0001 && 
            -0.0001 < x[3][2]  && x[3][2] < 0.0001 ); 
}

void Matrix::transpose()
{
    //printf("Matrix::transpose()\n");
   for (int i = 0; i < 4; i++)
      for (int j = 0; j < i; j++)
      {
          //printf("swapping (%d, %d) with (%d, %d)\n", i, j, j, i);
          double temp = x[i][j];
          x[i][j] = x[j][i];
          x[j][i] = temp;
      }
}

Matrix Matrix::getTranspose()const
{
   Matrix ret = *this;
   ret.transpose();

   return ret;
}

Vector3 transformLoc(const Matrix& left_op, const Vector3& right_op)
{ return left_op * right_op; }

Vector3 transformVec(const Matrix& left_op, const Vector3& right_op)
{
   Vector3 ret;

   ret[0] = right_op[0] * left_op.x[0][0] + right_op[1] * left_op.x[0][1] +
            right_op[2] * left_op.x[0][2];
   ret[1] = right_op[0] * left_op.x[1][0] + right_op[1] * left_op.x[1][1] +
            right_op[2] * left_op.x[1][2];
   ret[2] = right_op[0] * left_op.x[2][0] + right_op[1] * left_op.x[2][1] +
            right_op[2] * left_op.x[2][2];

   return ret;

}

Matrix zeroMatrix ()
{
   Matrix ret;
   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         ret.x[i][j] = 0.0;
   return ret;
}

Matrix identityMatrix()
{
   Matrix ret;
   ret.x[0][0] = 1.0;
   ret.x[0][1] = 0.0;
   ret.x[0][2] = 0.0;
   ret.x[0][3] = 0.0;
   ret.x[1][0] = 0.0;
   ret.x[1][1] = 1.0;
   ret.x[1][2] = 0.0;
   ret.x[1][3] = 0.0;
   ret.x[2][0] = 0.0;
   ret.x[2][1] = 0.0;
   ret.x[2][2] = 1.0;
   ret.x[2][3] = 0.0;
   ret.x[3][0] = 0.0;
   ret.x[3][1] = 0.0;
   ret.x[3][2] = 0.0;
   ret.x[3][3] = 1.0;
   return ret;
}

Matrix translate (double _x, double _y, double _z)
{
   Matrix ret = identityMatrix();

   ret.x[0][3] = _x;
   ret.x[1][3] = _y;
   ret.x[2][3] = _z;

   return ret;
}

// angle is in radians
Matrix rotateX (double angle)
{
   Matrix ret = identityMatrix();

   double cosine = cos(angle);
   double sine   = sin(angle);

   ret.x[1][1] = cosine;
   ret.x[1][2] =  -sine;
   ret.x[2][1] =   sine;
   ret.x[2][2] = cosine;

   return ret;
}

// angle is in radians
Matrix rotateY (double angle)
{
   Matrix ret = identityMatrix();

   double cosine = cos(angle);
   double sine   = sin(angle);

   ret.x[0][0] = cosine;
   ret.x[0][2] =   sine;
   ret.x[2][0] =  -sine;
   ret.x[2][2] = cosine;

   return ret;
}

// angle is in radians
Matrix rotateZ (double angle)
{
   Matrix ret = identityMatrix();

   double cosine = cos(angle);
   double sine   = sin(angle);

   ret.x[0][0] = cosine;
   ret.x[0][1] =  -sine;
   ret.x[1][0] =   sine;
   ret.x[1][1] = cosine;

   return ret;
}

// rotation is in radians about an arbitrary axis
Matrix rotate(const Vector3 & axis, double angle)
{
   Vector3 _axis = unitVector(axis);
   Matrix ret;
   double x = _axis.x();
   double y = _axis.y();
   double z = _axis.z();
   double cosine = cos(angle);
   double sine = sin(angle);
   double t   = 1 - cosine;

   ret.x[0][0] = t * x * x + cosine;
   ret.x[0][1] = t * x * y - sine * z;
   ret.x[0][2] = t * x * z + sine * y;
   ret.x[0][3] = 0.0;

   ret.x[1][0] = t * x * y + sine * z;
   ret.x[1][1] = t * y * y + cosine;
   ret.x[1][2] = t * y * z - sine * x;
   ret.x[1][3] = 0.0;

   ret.x[2][0] = t * x * z - sine * y;
   ret.x[2][1] = t * y * z + sine * x;
   ret.x[2][2] = t * z * z + cosine;
   ret.x[2][3] = 0.0;

   ret.x[3][0] = 0.0;
   ret.x[3][1] = 0.0;
   ret.x[3][2] = 0.0;
   ret.x[3][3] = 1.0;

   return ret;
}

Matrix scale(double _x, double _y, double _z)
{
   Matrix ret(zeroMatrix());

   ret.x[0][0] = _x;
   ret.x[1][1] = _y;
   ret.x[2][2] = _z;
   ret.x[3][3] = 1.0;

   return ret;
}

Matrix
lookAt(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
{
   Matrix ret;

   // create a viewing matrix
   
   Vector3 w = unitVector(eye - at);
   Vector3 u = unitVector(cross(up, w));
   Vector3 v = cross(w, u);
   
   ret.x[0][0] = u.x();
   ret.x[0][1] = u.y();
   ret.x[0][2] = u.z();
   ret.x[1][0] = v.x();
   ret.x[1][1] = v.y();
   ret.x[1][2] = v.z();
   ret.x[2][0] = w.x();
   ret.x[2][1] = w.y();
   ret.x[2][2] = w.z();

   // translare eye to xyz origin
   Matrix move; 
   move.x[0][3] = -(eye.x());
   move.x[1][3] = -(eye.y());
   move.x[2][3] = -(eye.z());
   
   ret = ret * move;
   ret.invert();
   return ret;
}

Matrix rtMatrixToGalileoMatrix(const float rt_matrix[4][4])
{
   Matrix ret;
   ret.x[0][0] = rt_matrix[0][0];
   ret.x[0][1] = rt_matrix[1][0];
   ret.x[0][2] = rt_matrix[2][0];
   ret.x[0][3] = rt_matrix[3][0];
   ret.x[1][0] = rt_matrix[0][1];
   ret.x[1][1] = rt_matrix[1][1];
   ret.x[1][2] = rt_matrix[2][1];
   ret.x[1][3] = rt_matrix[3][1];
   ret.x[2][0] = rt_matrix[0][2];
   ret.x[2][1] = rt_matrix[1][2];
   ret.x[2][2] = rt_matrix[2][2];
   ret.x[2][3] = rt_matrix[3][2];
   ret.x[3][0] = rt_matrix[0][3];
   ret.x[3][1] = rt_matrix[1][3];
   ret.x[3][2] = rt_matrix[2][3];
   ret.x[3][3] = rt_matrix[3][3];
   return ret;
}

Matrix operator+ (const Matrix & left_op, const Matrix & right_op)
{
   Matrix ret;

   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         ret.x[i][j] = left_op.x[i][j] + right_op.x[i][j];

   return ret;
}

Matrix operator- (const Matrix & left_op, const Matrix & right_op)
{
   Matrix ret;

   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         ret.x[i][j] = left_op.x[i][j] - right_op.x[i][j];

   return ret;
}

Matrix operator* (const Matrix & left_op, double right_op)
{
   Matrix ret;

   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         ret.x[i][j] = left_op.x[i][j] * right_op;

   return ret;
}

Matrix operator* (const Matrix & left_op, const Matrix & right_op)
{
   Matrix ret;

   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
      {
         double subt = 0.0;
         for (int k = 0; k < 4; k++)
            subt += left_op.x[i][k] * right_op.x[k][j];
         ret.x[i][j] = subt;
      }

   return ret;
}

// transform a vector by matrix
Vector3 operator* (const Matrix & left_op, const Vector3 & right_op)
{
   Vector3 ret;
   double temp;

   ret[0] = right_op[0] * left_op.x[0][0] + right_op[1] * left_op.x[0][1] +
            right_op[2] * left_op.x[0][2] +               left_op.x[0][3];
   ret[1] = right_op[0] * left_op.x[1][0] + right_op[1] * left_op.x[1][1] +
            right_op[2] * left_op.x[1][2] +               left_op.x[1][3];
   ret[2] = right_op[0] * left_op.x[2][0] + right_op[1] * left_op.x[2][1] +
            right_op[2] * left_op.x[2][2] +               left_op.x[2][3];
   temp   = right_op[0] * left_op.x[3][0] + right_op[1] * left_op.x[3][1] +
            right_op[2] * left_op.x[3][2] +               left_op.x[3][3];

   temp = 1.0 / temp;
   ret *= temp;
   return ret;
}

std::ostream & operator<< (std::ostream & out, const Matrix & right_op)
{
   for (int i = 0; i < 4; i++)
   {
      for (int j = 0; j < 4; j++)
         out << std::setprecision(3) << std::setw(10) << right_op.x[i][j];
      out << std::endl;
   }
   return out;
}

double Matrix::determinant()
{
   double det;
   det  = x[0][0] * det3(x[1][1], x[1][2], x[1][3],
                         x[2][1], x[2][2], x[2][3],
                         x[3][1], x[3][2], x[3][3]);
   det -= x[0][1] * det3(x[1][0], x[1][2], x[1][3],
                         x[2][0], x[2][2], x[2][3],
                         x[3][0], x[3][2], x[3][3]);
   det += x[0][2] * det3(x[1][0], x[1][1], x[1][3],
                         x[2][0], x[2][1], x[2][3],
                         x[3][0], x[3][1], x[3][3]);
   det -= x[0][3] * det3(x[1][0], x[1][1], x[1][2],
                         x[2][0], x[2][1], x[2][2],
                         x[3][0], x[3][1], x[3][2]);
   return det;
}

