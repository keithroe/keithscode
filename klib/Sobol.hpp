
// Copyright (C) 2011 R. Keith Morley 
// 
// (MIT/X11 License)
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef LEGION_COMMON_MATH_SOBOL_HPP_
#define LEGION_COMMON_MATH_SOBOL_HPP_

#include <Legion/Common/Math/Vector.hpp>

namespace legion
{


class Sobol
{
public:
    enum Dimension
    {
        DIM_PIXEL_X = 0, // Pixel dims must be 0,1
        DIM_PIXEL_Y,
        DIM_BSDF_X,
        DIM_BSDF_Y,
        DIM_LENS_X,
        DIM_LENS_Y,
        DIM_TIME,
        DIM_SHADOW_X,
        DIM_SHADOW_Y,
        NUM_DIMS
    };
    
    /// Generate a single element A_i,dim of the Sobol sequence which is the
    /// dimth dimension of the ith sobol vector. Scrambled by s. 
    static float    gen ( unsigned i, unsigned dim,  unsigned scramble = 0u );
    static unsigned genu( unsigned i, unsigned dim,  unsigned scramble = 0u );


    static const unsigned MAX_DIMS = 256u;
    static const unsigned MATRICES[ MAX_DIMS*52u ];
private:
    static float intAsFloat( int x );

    Sobol();
};



inline float Sobol::gen( unsigned i, unsigned dim,  unsigned scramble )
{
    return intAsFloat( 0x3F800000 | ( genu( i, dim, scramble ) >> 9 ) ) - 1.0f;
}


inline float Sobol::intAsFloat( int x )
{
  return *(float*)&x;
}


}

#endif //  LEGION_COMMON_MATH_SOBOL_HPP_
