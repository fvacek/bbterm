#ifndef BBTERM_CORE_UTIL_LOG_H
#define BBTERM_CORE_UTIL_LOG_H
 
#include <QDebug>

#ifndef NO_BBTERM_LOG_DEBUG
	#define LOGDEB() qDebug() << "[D]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#else
	#define LOGDEB() while(false) qDebug() << "[D]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#endif

#define LOGWARN() qWarning() << "[W]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))
#define LOGERR() qCritical() << "[E]" << (QString("%1:%2").arg(__FILE__).arg(__LINE__))

#endif // BBTERM_CORE_UTIL_LOG_H

