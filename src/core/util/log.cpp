#include "log.h"

using namespace core::util;


int Log::nextSerial()
{
	static int n = 0;
	return ++n;
}
