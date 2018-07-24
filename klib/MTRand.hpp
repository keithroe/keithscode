
// Small changes to fit into legion style, changing unsigned long
// long to uint64_t, adding float operator()(), etc.
// 2012, r. keith morley
//
// Mersenne Twister pseudorandom number generator of 32 and 64-bit.
// 2005, Diego Park <diegopark@gmail.com>
// 
// Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. The names of its contributors may not be used to endorse or promote
// products derived from this software without specific prior written
// permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// References:
// T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
// ACM Transactions on Modeling and 
// Computer Simulation 10. (2000) 348--357.
// M. Matsumoto and T. Nishimura,
// ``Mersenne Twister: a 623-dimensionally equidistributed
// uniform pseudorandom number generator''
// ACM Transactions on Modeling and 
// Computer Simulation 8. (Jan. 1998) 3--30.
// 
// Any feedback is very welcome.
// http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
// email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)

#include <cstddef> //  size_t
#include <stdint.h> // uint32_t 

#ifndef LEGION_COMMON_MATH_MTRAND_HPP_
#define LEGION_COMMON_MATH_MTRAND_HPP_


namespace legion
{

template <class Integer>
class MTRand
{
public:
    MTRand( Integer aSeed = 5489u );

    Integer next();
    float operator()() { return next() * ( 1.0f / 4294967296.0f ); }

private:
    static const size_t MaxState = 624 * sizeof(uint32_t) / sizeof(Integer);
    static const size_t Period;

    void seed();

    static Integer hashFunction(Integer anInteger);
    static Integer twist(Integer a, Integer b, Integer c);

    Integer state[MaxState];
    Integer index;
};

typedef MTRand<uint32_t>   MTRand32;
typedef MTRand<uint64_t>   MTRand64;


template <class Integer>
Integer MTRand<Integer>::next()
{
    if (index >= MaxState)
        seed();
    return hashFunction(state[index++]);
}


template <class Integer>
void MTRand<Integer>::seed()
{
    index = 0;

    size_t i;
    for (i = 0; i < MaxState - Period; i++)
        state[i] = twist(state[i + Period], state[i], state[i + 1]);

    for (; i < MaxState - 1; i++)
        state[i] = twist(state[i + (Period - MaxState)], state[i], state[i + 1]);

    state[MaxState - 1] = twist(state[Period - 1], state[MaxState - 1], state[0]);
}


}


#endif // LEGION_COMMON_MATH_MTRAND_HPP_
