#include "Logger.h"


void Log(int iLevel, const char* szFormat, ...)
{
	va_list va;
	va_start(va, szFormat);

	vsyslog(iLevel, szFormat, va);

	va_end(va);
}

