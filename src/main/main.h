#ifndef __MAIN_H__
#define __MAIN_H__

#define MAXCONNECT  10
#define MAXLINE     1500
#define MAXDEVNUM   255
#define MAXNUM      10000
#define SERV_PORT   10000

#define LED_RUN   124
#define LED_ERR   123
//#define IEC10XLOCK

//watch dog
#define WATCHDOG_IOCTL_BASE     'W'

#define WDIOC_GETSUPPORT        _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_GETSTATUS         _IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define WDIOC_GETBOOTSTATUS     _IOR(WATCHDOG_IOCTL_BASE, 2, int)
#define WDIOC_GETTEMP           _IOR(WATCHDOG_IOCTL_BASE, 3, int)
#define WDIOC_SETOPTIONS        _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE         _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT        _IOR(WATCHDOG_IOCTL_BASE, 7, int)
#define WDIOC_SETPRETIMEOUT     _IOWR(WATCHDOG_IOCTL_BASE, 8, int)
#define WDIOC_GETPRETIMEOUT     _IOR(WATCHDOG_IOCTL_BASE, 9, int)
#define WDIOC_GETTIMELEFT       _IOR(WATCHDOG_IOCTL_BASE, 10, int)

#define WDIOF_OVERHEAT          0x0001  /* Reset due to CPU overheat */
#define WDIOF_FANFAULT          0x0002  /* Fan failed */
#define WDIOF_EXTERN1           0x0004  /* External relay 1 */
#define WDIOF_EXTERN2           0x0008  /* External relay 2 */
#define WDIOF_POWERUNDER        0x0010  /* Power bad/power fault */
#define WDIOF_CARDRESET         0x0020  /* Card previously reset the CPU */
#define WDIOF_POWEROVER         0x0040  /* Power over voltage */
#define WDIOF_SETTIMEOUT        0x0080  /* Set timeout (in seconds) */
#define WDIOF_MAGICCLOSE        0x0100  /* Supports magic close char */
#define WDIOF_PRETIMEOUT        0x0200  /* Pretimeout (in seconds), get/set */
#define WDIOF_KEEPALIVEPING     0x8000  /* Keep alive ping reply */

#define WDIOS_DISABLECARD       0x0001  /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD        0x0002  /* Turn on the watchdog timer */
#define WDIOS_TEMPPANIC         0x0004  /* Kernel panic on temperature trip */



extern int listenfd,connfd;

extern uint8_t TCP_STATE;//tcp ����״̬,0:δ���ӣ�1:������
extern YK_INFO_T Device_Yk_Info[MAX_SLAVE_DEVICE_NUM];//ң����Ϣ


extern pthread_mutex_t mutex;


#define VERSION     0.1
#define NAME        Iec104_monitor



typedef struct{

    int socketfd;
    unsigned short int port;
    unsigned int addr;

}SocketArg_T, *PSocketArg_T;



typedef struct{
    int socketfd;
    uint16_t ClientId;
	int *Connect_State;
	uint8_t *master_flag;
	int port;
	char *Dstip;
	SendRecvSn_T SendRecvSn;
	uint8_t slave_device_num;
	YKReturn_T YK_Return;
}ClientInfo_T,*PClientInfo_T;

typedef struct{
//	char	ComType[32];
    int 	DevID;
	int 	fd;

}TaskInfo_T,*PTaskInfo_T;


enum {
	GX104MasterID,		/*0*/
	GX104SlaverID,		/*1*/
	GX101MasterID,		/*2*/
	GX101SlaverID,		/*3*/
	siec101gxID,		/*4*/
	miec104gxID,		/*5*/
	siec104_2002nrID,	/*6*/
	MonitorID,			/*7*/
	XJ103ID,			/*8*/
};


int UART0_Recv(int fd, char *rcv_buf,int data_len);    



#endif
