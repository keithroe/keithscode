
#ifndef KLIB_LOGGER_H_
#define KLIB_LOGGER_H_

//
// Logger class which strives for:
//   - Efficient handling of below-threshold logging events
//   - Simplicity
//
// Usage:
//   - Set compile-time debug level via preprocessor macro KLOG_MAX_LEVEL
//     All logging events > KLOG_MAX_LEVEL will be eliminated by compiler
//   - Set runtime debug level via Log::setReportingLevel()
//   - Use  KLOG macro to log at desired level:
//     KLOG( Log::WARNING ) << "Some warning message";
//     The logger will insert newlines after each message
//
// Inspired by http://drdobbs.com/cpp/201804215
//

#include <iostream>
#include <ctime>
#include <sys/time.h>


#ifndef KLOG_MAX_LEVEL
#   define KLOG_MAX_LEVEL Log::WARNING
#endif


#define KLOG( level )                                                          \
    if( level > KLOG_MAX_LEVEL)                 ;                              \
    else if( level > Log::getReportingLevel() ) ;                              \
    else Log().get( level )


class Log
{
public:
    enum Level
    {
        ERROR=0,
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
    static std::ostream& s_out;
};

std::ostream& Log::s_out             = std::cerr;
Log::Level    Log::s_reporting_level = Log::WARNING;


//
inline Log::Log()
{
}


inline Log::~Log()
{
    s_out << "\n";
}


inline std::ostream& Log::get( Log::Level level )
{
    s_out << "[" << time() << "] " << toString( level ) << ": "
          << std::string( (level > INFO ? level - INFO : 0 ), '\t' );
    return s_out;
}


inline Log::Level Log::getReportingLevel()
{
    return s_reporting_level;
}


inline void Log::setReportingLevel( Level level )
{
    s_reporting_level = level;
}


std::string Log::toString( Log::Level level )
{
    static const char* const level2string[] = 
    {
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


#endif // KLIB_LOGGER_H_
