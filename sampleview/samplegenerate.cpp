
#include <cstdlib>
#include <iostream>

typedef unsigned int uint;

double RI_vdC(uint bits, uint r = 0)
{
    bits = ( bits << 16)
         | ( bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8)
         | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4)
         | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2)
         | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1)
         | ((bits & 0xaaaaaaaa) >> 1);
    bits ^= r;
    return (double) bits / (double) 0x100000000LL;
}


double RI_S(uint i, uint r = 0)
{
    for(uint v = 1<<31; i; i >>= 1, v ^= v>>1)
        if(i & 1) r ^= v;
    return (double) r / (double) 0x100000000LL;
}


double RI_LP(uint i, uint r = 0)
{
    for(uint v = 1<<31; i; i >>= 1, v |= v>>1)
        if(i & 1) r ^= v;
    return (double) r / (double) 0x100000000LL;
}


int main(int argc, char** argv )
{
    int sqrt_samples = atoi( argv[1] );
    int scramble     = atoi( argv[2] ); 
    for( int i = 0; i < sqrt_samples*sqrt_samples; ++i )
    {
        //std::cout << RI_vdC(i, 139398356) << " " << RI_S(i, 99883923) << " ";
        //std::cout << drand48() << " " << drand48() << " ";
        std::cout << static_cast<float>(i) / (sqrt_samples*sqrt_samples) << " " << RI_LP(i, scramble ) << " ";
    }
    std::cout << std::endl;
}
