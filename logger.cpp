#include <sys/syscall.h>
#include <cassert>
#include "logger.h"

//日志级别控制变量，该变量指向共享内存。该内存的值由其他进程修改。
int *g_pshmLogLevel = NULL;

int *g_pSuggestedLevel = NULL;
int g_uEpollThr = 65535; //足够大了

zlog_category_t* category1 = NULL;
zlog_category_t* category2 = NULL;
//检测是否输出日志
#define IS_NOT_DISPLAY_LOG(value)   ( (g_pshmLogLevel != NULL) && (value > *g_pshmLogLevel) )

static bool bDiskFull = false;

static int zlogLevel[] =
{
	ZLOG_LEVEL_FATAL,
	ZLOG_LEVEL_ERROR,
	ZLOG_LEVEL_ERROR,
	ZLOG_LEVEL_ERROR,
	ZLOG_LEVEL_WARN,
	ZLOG_LEVEL_NOTICE,
	ZLOG_LEVEL_INFO,
	ZLOG_LEVEL_DEBUG
};


void initLog(const char *confpath, const char *cname,const char* cname1, const char* cname2)
{
	dzlog_init(confpath, cname);

	// Find log file path from category
	zlog_category_t* c = zlog_get_category((char *)cname);

    category1 = zlog_get_category((char *)cname1);
    category2 = zlog_get_category((char *)cname2);

	zc_arraylist_t* a_list = c->fit_rules;
	assert(a_list->len == 1);

	//zlog_rule_t* a_rule = (zlog_rule_t *)a_list->array[0];
}


#define PRINT_THREAD_ID

void log(int l, const char *fmt, ...)
{
	if (bDiskFull)
		return;

	va_list		param;
	{
		va_start(param, fmt);
		int rc = vdzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, zlogLevel[l], fmt, param);
		if (rc == -1 && errno == ENOSPC)
		{
			// 磁盘空间满，后续日志调用暂停
			bDiskFull = true;
		}
	}
	va_end(param);
}

void OUTLOG(INTERFACE_LOG& outLog, uint32_t stat)
{
    outLog.setStat(stat);
    outputLog(Info, "%lu|%s|%lx|%u|%u|%s|%s|2|%u||||||%u", getTimeMs(), outLog.ip.c_str(), outLog.uid64, outLog.uid, outLog.requestId, outLog.module, outLog.interface, stat, outLog.towardId);
}

void INLOG(INTERFACE_LOG& inLog)
{
    outputLog(Info, "%lu|%s|%lx|%u|%u|%s|%s|1|0||||||%u", getTimeMs(), inLog.ip.c_str(), inLog.uid64,  inLog.uid, inLog.requestId, inLog.module, inLog.interface, inLog.towardId);
}

void outputLog(int l, const char *fmt, ...)
{
	va_list	param;

    va_start(param, fmt);
    vzlog(category1, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, zlogLevel[l], fmt, param);
  
	va_end(param);
}

void logPriceInfo(int l, const char *fmt, ...)
{
	va_list	param;

    va_start(param, fmt);
    vzlog(category2, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, zlogLevel[l], fmt, param);
  
	va_end(param);
}
uint64_t getTimeMs()
{
	struct timeval tmpTm;
	gettimeofday(&tmpTm, NULL);

	return (uint64_t)(tmpTm.tv_sec * 1000000 + tmpTm.tv_usec)/1000;
}


char *ip2str(uint32_t ip)
{
	union ip_addr{
uint32_t addr;
uint8_t s[4];
} a;
a.addr = ip;
static char s[16];
sprintf(s, "%u.%u.%u.%u", a.s[0], a.s[1], a.s[2], a.s[3]);
return s;
}
