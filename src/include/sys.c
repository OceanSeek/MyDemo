#include "sys.h"

char* sysLocalTime()  
{  
    time_t             timesec;  
    struct tm         *p;  
	char *timestamp;
	struct timeval	  tv;  
	gettimeofday(&tv, NULL);  
//	  printf("tv_usec:%ld\n",tv.tv_usec);  
	

    time(&timesec);
    p = localtime(&timesec);  

	timestamp = malloc(21);
	sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d.%03d", 1900+p->tm_year, p->tm_mon + 1,\
		p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec/1000);
	return timestamp;

}  

void LogSysLocalTime()  
{  
    time_t             timesec;  
    struct tm         *p;  
	char *timestamp;

    time(&timesec);
    p = localtime(&timesec);  

	log("%d-%02d-%02d %02d:%02d:%02d\n", 1900+p->tm_year, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}  

int HuaWei485Ctrl_Switch(int DevNo, int ctrlvalue)
{
	char *CtrlPort;
	int iDataLen, crlfd;
	switch(gpDevice[DevNo].SerialID){
		case 0:
			CtrlPort = "/dev/rs485Ctrl1";
			break;
		case 1:
			CtrlPort = "/dev/rs485Ctrl2";
			break;
		default:
			perror("no such port");
			return -1;
	}
	crlfd = open(CtrlPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (crlfd < 0){ 
        printf("open file %s fail \r\n", CtrlPort);        
        return -1;
    } 
	
	iDataLen = write(crlfd, &ctrlvalue, sizeof(ctrlvalue));
	if (0 > iDataLen){
		printf("write  file %s fail \r\n", CtrlPort);
		return -1;
	}

	close(crlfd);

}

uint32_t ip2long(char* ip)
{
	// exsample ip = "192.168.1.21"
	uint8_t addr[4] = {0};
	sscanf(ip, "%d.%d.%d.%d", &addr[0],&addr[1],&addr[2],&addr[3]);
	return *(uint32_t*)addr;
}



