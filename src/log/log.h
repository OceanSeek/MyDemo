#ifndef __LOG_H__
#define __LOG_H__

#define MAX_LOG_FILE     500000

#define LOGDEBUG		0
#define LOGMARNING		1
#define LOGERROR		2
#define STARTINFO		3

int write_log(char* fmt, int type, const char *file, const char *func, uint32_t line);

#endif

