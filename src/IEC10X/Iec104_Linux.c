/*******************************************************************
Copyright (C):    
File name    :    HXCP_STM32.C
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/02/26
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2014/02/26         Author: ChenDajie
   content:

*******************************************************************/
#include "sys.h"


/*******************************************************************************
* Function Name  : stm32f103_init
* Description    : stm32f103_init program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static int Linux_init(void) {


    return 0;
}

static void *Linux_Malloc(unsigned char NumByte){

    return malloc(NumByte);
}


static void Linux_Free(void *pblock){

    free(pblock);
}

//struct tm   
//{   
//    int tm_sec; //当前秒    
//    int tm_min; //当前分钟    
//    int tm_hour; //当前小时    
//    int tm_mday; //当前在本月中的天，如11月1日，则为1    
//    int tm_mon; //当前月，范围是0~11    
//    int tm_year; //当前年和1900的差值，如2006年则为36    
//    int tm_wday; //当前在本星期中的天，范围0~6    
//    int tm_yday; //当前在本年中的天，范围0~365    
//    int tm_isdst; //这个我也不清楚    
//}   
//uint16_t _milliseconds;
//IEC10X_Time_Min_T _min;
//IEC10X_Time_Hour_T _hour;
//IEC10X_Time_Day_T _day;
//IEC10X_Time_Month_T _month;
//IEC10X_Time_Year_T _year;


static uint32_t Linux_SetTime(PCP56Time2a_T time){
	struct tm _tm;	
	struct timeval tv;	
	time_t timep;  
	
	_tm.tm_sec = time->_milliseconds/1000;  
	_tm.tm_min = time->_min._minutes;  
	_tm.tm_hour = time->_hour._hours;  
	_tm.tm_mday = time->_day._dayofmonth;  
	_tm.tm_mon = time->_month._month - 1;  
	_tm.tm_year = time->_year._year + 100;  
	timep = mktime(&_tm);  
	tv.tv_sec = timep;	
	tv.tv_usec = 0;  
	if(settimeofday (&tv, (struct timezone *) 0) < 0)  
	{  
		log("Set system datatime error!\n");  
		return -1;	
	}  
	system("hwclock -w");//将系统时间写入硬件时钟
	log("Set system datatime suceess!\n");  

    return RET_SUCESS;
}
static uint32_t Linux_GetTime(PCP56Time2a_T TimeValue){
    time_t timer;   
    struct tm* t_tm;   
	
    time(&timer);   
    t_tm = localtime(&timer);   
//    printf("today is %4d-%02d-%02d %02d:%02d:%02d\n", 
//    	t_tm->tm_year+1900, t_tm->tm_mon+1, t_tm->tm_mday, 
//    	t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec); 

    struct timeval    tv;  
    gettimeofday(&tv, NULL);  
//    printf("tv_usec:%ld\n",tv.tv_usec);  

	
 


	TimeValue->_milliseconds = t_tm->tm_sec*1000 + tv.tv_usec/1000;  
	TimeValue->_min._minutes = t_tm->tm_min;  
	TimeValue->_hour._hours = t_tm->tm_hour;  
	TimeValue->_day._dayofmonth = t_tm->tm_mday;  
	TimeValue->_month._month = t_tm->tm_mon + 1;  
	TimeValue->_year._year = t_tm->tm_year - 100;  
//    printf("_milliseconds:%d\n",TimeValue->_milliseconds);  

    return RET_SUCESS;
}


static int8_t Linux_GetStationInfo(uint16_t *Addr,uint8_t n,uint8_t *MaxNum){

    *MaxNum = WTP_SUPPORT_END_NUMBER;
    *Addr = n;
    return RET_SUCESS;
}

static float Linux_GetStaValue(uint16_t Addr, uint8_t DevType){
	
}

static uint16_t Linux_GetLinkAddr(void){

    return 0x0001;
}

static void Linux_CloseLink(void){

    //GPRSFlag = SYSTEM_FLAG_GPRS_CLOSE;
    //IEC104_STATE_FLAG_INIT = IEC104_FLAG_CLOSED;
}

void delay_ms(uint16_t ms){

    usleep(ms*1000);
}

int Linuxsend(int socketfd,char *buf, int len){

    if(-1 == write(socketfd,buf,len)){
        LOG("-%s-,Send error fd(%d)\n",__FUNCTION__,socketfd);
        return RET_ERROR;
    }
//    printf("Send Ok!\r\n");
    
    return RET_SUCESS;
}
//获取遥信数量，数量返回InfoNum
uint16_t Linux_GetInfoNum(uint16_t *InfoNum, uint8_t DevType){


	
	return RET_SUCESS;
}



void LinuxLock(void){

//    pthread_mutex_lock(&mutex);
}
void LinuxUnlock(){

//    pthread_mutex_unlock(&mutex);
}
/*******************************************************************************
* Function Name  : stm32f103
* Description    : stm32f103
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static IEC10X_T Linux = {
    "Linux",
    Linux_init,
    delay_ms,
    Linux_CloseLink,
    Linux_Malloc,
    Linux_Free,
    IEC10X_PrioEnQueue,
    IEC10X_PrioDeQueue,
    IEC10X_PrioFindQueueHead,
    IEC10X_HighestPrio,            /* Get the highest Prio Queue*/
    IEC10X_PrioInitQueue,
    IEC10X_Prio_ClearQueue,
    Linuxsend,
    Linux_SetTime,
    Linux_GetTime,
    Linux_GetStationInfo,
    Linux_GetStaValue,
    Linux_GetLinkAddr,
    Linux_GetInfoNum,
    

#ifdef IEC10XLOCK
    LinuxLock,
    LinuxUnlock
#endif
};
/*******************************************************************************
* Function Name  : mstm32f103RegisterShtxxain
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t Stm32f103RegisterIec10x(void){
    return RegisterIEC10XMoudle(&Linux);
}
