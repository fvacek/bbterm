#ifndef BBTERM_CORE_UTIL_LOG_H
#define BBTERM_CORE_UTIL_LOG_H
 
#include <QDebug>

namespace core {
namespace util {

class Log
{
public:
	static int nextSerial();
};

}
}

#ifndef NO_BBTERM_LOG_DEBUG
	#define LOGDEB() qDebug() << core::util::Log::nextSerial() << "[D]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#else
	#define LOGDEB() while(false) qDebug() << core::util::Log::nextSerial() << "[D]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#endif

#define LOGWARN() qWarning() << core::util::Log::nextSerial() << "[W]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#define LOGERR() qCritical() << core::util::Log::nextSerial() << "[E]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))

#endif // BBTERM_CORE_UTIL_LOG_H

