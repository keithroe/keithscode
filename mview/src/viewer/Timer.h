/************************************************************************\
 *                                                                      *
 * filename: timer.h                                                    *
 * author  : R. Keith Morley                                            *
 * lastmod : 12/27/01                                                   *
 *                                                                      *
 * timer is a simple program timer class implemented using the standard *
 * library sys/time.h and unistd.h                                      *
 *                                                                      *
\************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_ 1

#include <sys/time.h>
#include <unistd.h>
#include <cstdio>


class Timer
{
public:
   Timer();
   void reset();
   long int get();
   long int getMicro();
   long int getMicroReset();
   long int getReset();

private:
   struct timeval tv;
};

inline
void printTime (long int the_time)
{ printf("%22.3fs", ((double)(the_time)) / 1000.0); }

inline
void Timer::reset() { gettimeofday(&tv, NULL); }

inline
Timer::Timer() { reset(); }

inline
long int Timer::get()
{
   struct timeval tv2; gettimeofday(&tv2, NULL);
   return (tv2.tv_usec - tv.tv_usec) / 1000 + (tv2.tv_sec - tv.tv_sec) * 1000;
}

inline
long int Timer::getMicro()
{
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   return (tv2.tv_usec - tv.tv_usec) + (tv2.tv_sec - tv.tv_sec)*1000000;
}

inline
long int Timer::getMicroReset()
{
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   long ret = (tv2.tv_usec - tv.tv_usec) + (tv2.tv_sec - tv.tv_sec)*1000000;
   tv.tv_sec = tv2.tv_sec; tv.tv_usec = tv2.tv_usec;
   return ret;
}

inline
long int Timer::getReset()
{
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   long ret = (tv2.tv_usec - tv.tv_usec)/1000 + (tv2.tv_sec - tv.tv_sec)*1000;
   tv.tv_sec = tv2.tv_sec; tv.tv_usec = tv2.tv_usec;
   return ret;
}

#endif // _TIMER_H_


