
#ifndef __SYS_H
#define __SYS_H	

#include<sys/stat.h>       
#include <termios.h>       
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include "unistd.h"
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<net/if.h>
#include<string.h>
#include <pthread.h>

//#include <iostream>

#include "MQTTClient.h"
#include "log.h"
#include "common_datatype.h"
#include "cJSON.h"
#include "Iec10x.h"
#include "mbrtu.h"
#include "mbtcp.h"
#include "mbtcpclient.h"
#include "mbrtumaster.h"
#include "Iec104_Linux.h"
#include "Dbase.h"
#include "queue.h"
#include "PRIO_QUEUE_Iec10x.h"
#include "sqlite3.h"
#include "usart.h"
#include "ai.h"
#include "Ait.h"
#include "bi.h"
#include "ci.h"
#include "bcd.h"
#include "dbiSoe.h"
#include "dbi.h"
#include "soe.h"
#include "mr.h"
#include "monitor.h"
#include "SXJ103.h"
#include "GX104Master.h"
#include "GX104Slaver.h"
#include "ModbusRtuMaster.h"
#include "ModbusRtuSlaver.h"
#include "ModbusTcpMaster.h"
#include "ModbusTcpSlaver.h"
#include "siec101_2002.h"
#include "siec104_2002nr.h"
#include "miec104gx.h"
#include "MXJ103.h"
#include "GX101Slaver.h"
#include "GX101Master.h"
#include "siec101_2002.h"
#include "mqtt_Client.h"
#include "json_test.h"

#define HUAWEI
#define RS485CTL_Write 	0
#define RS485CTL_Read	1

char* sysLocalTime();  
void LogSysLocalTime(); 
int HuaWei485Ctrl_Switch(int DevNo, int ctrlvalue);
uint32_t ip2long(char* ip);


#define WTP_SUPPORT_END_NUMBER  9
#define Timer_500ms    500
#define Timer_1000ms    1000
#define Timer_2000ms    2000
#define Timer_4000ms    4000
#define Timer_2s    	2
#define Timer_60s   	60
#define FALSE  0    
#define TRUE   1

#define RESEND_YES	1
#define RESEND_NO  	0

#define RET_SUCESS  0
#define RET_ERROR   -1

//#define IEC101_STM32
#define IEC104_STM32
//#define IEC104_STM32_FUJIAN_HX

//#define SIM900_MODE_SERVER
#define SIM900_MODE_CLIENT


#define PERROR(fmt,argc...) printf("Error:Function(%s)line(%d)--- "fmt, __FUNCTION__, __LINE__, ##argc)
#define SERIAL_DEBUG//����log

//#define perror(...) printf("Error:(%s)line(%d)---(%s)  \n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define perror(fmt,argc...) printf("Error:Function(%s)line(%d)--- "fmt, __FUNCTION__, __LINE__, ##argc)


#ifdef SERIAL_DEBUG
#define log(...) printf(__VA_ARGS__)
#define LOG(...) printf(__VA_ARGS__)
//#define perror(...) printf("Error:(%s)line(%d)---(%s)  \n", __FUNCTION__, __LINE__, __VA_ARGS__)
#define LogInit()
#define PRINT_FUNLINE printf("_________________line(%d),function(%s)\r\n",__LINE__,__FUNCTION__)
#else
#define log(...)
#define LOG(...)
//#define perror(...) 
//#define DumpHEX(buffer,len)
#define PRINT_FUNLINE
#endif


#ifdef  SERIAL_DEBUG
//void DumpHEX(uint8_t *buffer, uint32_t len);
#endif

#endif
