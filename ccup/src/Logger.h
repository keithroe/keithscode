//
// MIT License
//
// Copyright (c) 2008 r. keith morley 
//
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
// IN THE SOFTWARE
//

#include <iostream>
#include <string>
#include <ctime>
#include <sys/time.h>

#ifndef KLOG_MAX_LEVEL
#  ifdef LOCAL
#     define KLOG_MAX_LEVEL Log::DEBUG1
#  else
#     define KLOG_MAX_LEVEL Log::NONE
#  endif
#endif


#define KLOG( level )                                                          \
    if( level > KLOG_MAX_LEVEL)                 ;                              \
    else if( level > Log::getReportingLevel() ) ;                              \
    else Log().get( level )

#define LINFO   KLOG( Log::INFO )
#define LDEBUG  KLOG( Log::DEBUG )
#define LDEBUG1 KLOG( Log::DEBUG1 )

class Log
{
public:
    enum Level
    {
        NONE=0,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        DEBUG1,
        DEBUG2,
        DEBUG3,
        DEBUG4
    };

    //**************************************************************************
    // INFO: Do not use this public interface directly: use KLOG macro
    Log();
    ~Log();
    std::ostream& get( Level level = INFO );
    //
    //**************************************************************************

    static Level         getReportingLevel();
    static void          setReportingLevel( Level level );
    static void          setStream( std::ostream& out );

private:
    Log(const Log&);
    Log& operator=(const Log&);

    static std::string time();
    static std::string toString( Level level );
    
    static Level         s_reporting_level;
    static std::ostream* s_out;
};


//
inline Log::Log()
{
}


inline Log::~Log()
{
    *s_out << "\n";
}


inline std::ostream& Log::get( Log::Level level )
{
    *s_out << "[" << time() << "] " << toString( level ) << ": "
          << std::string( (level > INFO ? level - INFO : 0 ), '\t' );
    return *s_out;
}


inline Log::Level Log::getReportingLevel()
{
    return s_reporting_level;
}


inline void Log::setReportingLevel( Level level )
{
    s_reporting_level = level;
}

inline void Log::setStream( std::ostream& out )
{
    s_out = &out;
}

inline std::string Log::toString( Log::Level level )
{
    static const char* const level2string[] = 
    {
        "NONE",
        "ERROR",
        "WARNING",
        "INFO",
        "DEBUG",
        "DEBUG1",
        "DEBUG2",
        "DEBUG3",
        "DEBUG4"
    };
    return level2string[ level ];
}


inline std::string Log::time()
{
    char buffer[11];
    time_t t;
    std::time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));

    struct timeval tv;
    gettimeofday(&tv, 0);

    char result[100];
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}



