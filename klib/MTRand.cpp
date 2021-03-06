
// Small changes to fit into legion style, changing uint32_t
// long to uint64_t, etc.
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

#include "MTRand.hpp"

namespace legion
{

template <> const size_t MTRand<uint32_t>::Period = 397;

template <>
MTRand<uint32_t>::MTRand(uint32_t aSeed) {
	state[0] = aSeed & 0xffffffffUL;
	for (index = 1; index < MaxState; index++) {
		state[index] = 
			(1812433253UL * (state[index - 1] ^ (state[index - 1] >> 30)) + index) & 0xffffffffUL;
	}

	seed();
}

template <>
uint32_t MTRand<uint32_t>::hashFunction(uint32_t anInteger) {
	anInteger ^= (anInteger >> 11);
	anInteger ^= (anInteger << 7) & 0x9d2c5680UL;
	anInteger ^= (anInteger << 15) & 0xefc60000UL;
	anInteger ^= (anInteger >> 18);
	return anInteger;
}

template <>
uint32_t MTRand<uint32_t>::twist(uint32_t a, uint32_t b, uint32_t c) {
	return
		a ^
		(((b & 0x80000000UL) | (c & 0x7fffffffUL)) >> 1) ^
		(((c & 1) * 0xffffffffUL) & 0x9908b0dfUL);
}

template <> const size_t MTRand<uint64_t>::Period = 156;

template <>
MTRand<uint64_t>::MTRand(uint64_t aSeed) {
	state[0] = aSeed;
	for (index = 1; index < MaxState; index++) {
		state[index] = 
			6364136223846793005ULL * (state[index - 1] ^ (state[index - 1] >> 62)) + index;
	}

	seed();
}

template <>
uint64_t MTRand<uint64_t>::hashFunction(uint64_t anInteger) {
	anInteger ^= (anInteger >> 29) & 0x5555555555555555ULL;
	anInteger ^= (anInteger << 17) & 0x71D67FFFEDA60000ULL;
	anInteger ^= (anInteger << 37) & 0xFFF7EEE000000000ULL;
	anInteger ^= (anInteger >> 43);
	return anInteger;
}

template <>
uint64_t MTRand<uint64_t>::twist(uint64_t a, uint64_t b, uint64_t c) {
	return
		a ^
		(((b & 0xffffffff80000000ULL) | (c & 0x7fffffffULL)) >> 1) ^
		(((c & 1) * 0xffffffffffffffffULL) & 0xb5026f5aa96619e9ULL);
}
}
