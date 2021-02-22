#include "sys.h"
extern pthread_mutex_t mutex;

int write_log(char* fmt, int type, const char *file, const char *func, uint32_t line)
{
	char *filename1, *filename2;
	time_t timep;
	struct tm *p;
	
	char timebuf[100];
	char info[255];
	char *logType;
//	pthread_mutex_lock(&mutex);
	time(&timep);
	p = localtime(&timep);
	sprintf(timebuf, "%d/%02d/%02d %02d:%02d:%02d", (1900 + p->tm_year), (p->tm_mon+1), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
//	log("timebuf is %s \n",timebuf);
	filename1 = "log1.txt";
	filename2 = "log2.txt";
	FILE *fp = fopen(filename1, "a");//参数a往后追加
	if(!fp) return -1;
	fseek(fp, 0L, SEEK_END);
	uint32_t size = ftell(fp);
	switch (type)
	{
		case LOGDEBUG:
			logType = "[Debug]:  ";
			break;
		case LOGMARNING:
			logType = "[Marning]:";
			break;
		case LOGERROR:
			logType = "[Error]:  ";
			break;
		case STARTINFO:
			logType = "[StartInfo]:";
			break;
			
		default:
			break;
	}
	sprintf(info, "  file(%s)-Function(%s)-line(%d):", file, func, line);
	fprintf(fp, timebuf);
	fprintf(fp, logType);
	fprintf(fp, info);
	fprintf(fp, fmt);
	if(size > MAX_LOG_FILE){
		rename(filename1, filename2);
	}
	fclose(fp);
//	pthread_mutex_unlock(&mutex);
	return 1;

}

