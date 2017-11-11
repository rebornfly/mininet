#ifndef LOGGER_H_
#define LOGGER_H_

#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <sys/time.h>
#include <string>

#include <zlog.h>
#define MAXLEN_PATH 1024
#define MAXLEN_CFG_LINE (MAXLEN_PATH * 4)

struct INTERFACE_LOG
{
    INTERFACE_LOG(const char* pInterface, const char* pModel, uint64_t uUid64, std::string strIp, uint32_t uUid, uint32_t uResStat, uint32_t uRequestId, uint32_t uTowardId)
        :interface(pInterface),module(pModel),uid64(uUid64),uid(uUid),resStat(uResStat), requestId(uRequestId),towardId(uTowardId),ip(strIp)
    {
    }
    
    const char* interface;
    const char* module;
    uint64_t uid64;
    uint32_t uid;
    uint32_t resStat;
    uint32_t requestId;
    uint32_t towardId;
    std::string ip;

    void setStat(uint32_t stat)
    {
        resStat = stat;
    }

};

typedef struct {
	void **array;
	int len;
	int size;	
	void* del;
}zc_arraylist_t;
typedef struct zlog_rule_s {
	char category[MAXLEN_CFG_LINE + 1];	
	char compare_char;	
	/*	 
	* [*] log all level	 
	* [.] log level >= rule level, default	 
	* [=] log level == rule level	 
	* [!] log level != rule level	 
	*/	
	int level;	
	unsigned char level_bitmap[32]; 
	/* for category determine whether ouput or not */
	char file_path[MAXLEN_PATH + 1];
	zc_arraylist_t *dynamic_file_specs;	
	int static_file_descriptor;
} zlog_rule_t;

typedef struct zlog_category_s 
{	
	char name[MAXLEN_PATH + 1];
	size_t name_len;	
	unsigned char level_bitmap[32];	
	unsigned char level_bitmap_backup[32];
	zc_arraylist_t *fit_rules;	
	zc_arraylist_t *fit_rules_backup;
} zlog_category_t;

enum LogLevel
{
	Fatal = LOG_EMERG,
	Error = LOG_ERR,
	Warn = LOG_WARNING,
	Notice = LOG_NOTICE,
	Info = LOG_INFO,
	Debug = LOG_DEBUG
};

char *ip2str(uint32_t ip);
void initLog(const char *confpath, const char *cname, const char* cname1, const char* cname2);
void log(int l, const char *format,...) __attribute__((format(printf,2,3)));
void outputLog(int l, const char *fmt, ...) __attribute__((format(printf,2,3)));
void logPriceInfo(int l, const char *fmt, ...);
void OUTLOG(INTERFACE_LOG& outLog, uint32_t stat);
void INLOG(INTERFACE_LOG& inLog);
void setDiskFull(bool bFull);
uint64_t getTimeMs();

#define SYSLOG(level, fmt, ...)   log(level, "[%s::%s]: " fmt, __CLASS__, __FUNCTION__, ##__VA_ARGS__)

#endif // LOGGER_H_
