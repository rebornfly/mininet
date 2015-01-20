#pragma once 

#include <stdarg.h>
#include <stdint.h>
#include <syslog.h>

#define URI_PARSE(X) (X >> 8), (X & 0xFF)
	
enum LogLevel
{
	Error = 3,
	Warn = 4,
	Notice = 5,
	Info = 6,
	Debug = 7
};

void Log(int iLevel, const char* szFormat, ...);

