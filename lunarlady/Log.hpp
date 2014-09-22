#ifndef LL_LOG_HPP
#define LL_LOG_HPP

//#include <string>
#include <sstream>

namespace lunarlady {
	void Log(const std::string& iMessage, const std::string& iFunction, const std::string& iFile, const int iLine);
}

#define GENERAL_LOG(x) { ::std::ostringstream log_logStream; log_logStream << x; ::lunarlady::Log(log_logStream.str(), __FUNCTION__, __FILE__, __LINE__); }

#ifndef NODEBUG
#ifdef NDEBUG
// release
#define LOG1(x) GENERAL_LOG(x)
#define LOG2(x) if( false )
#define LOG3(x) if( false )
#else
// debug
#define LOG1(x) GENERAL_LOG(x)
#define LOG2(x) GENERAL_LOG(x)
#define LOG3(x) if( false )
#endif
#else
// no logging whatsoever
#define LOG1(x) if( false )
#define LOG2(x) if( false )
#define LOG3(x) if( false )
#endif

#endif