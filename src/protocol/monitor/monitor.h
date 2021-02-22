#ifndef __MONITOR_H__
#define __MONITOR_H__

#define SHOWIDSABLE 		0
#define SHOWENABLE 			1
#define IDDISABLE 			0xff
#define MAXBUFLEN 			1000

#define MONITOR_ENABLE 		1
#define MONITOR_DISABLE 	0
enum{	
	SET_RX_TX_ID,		/*0*/
	SHOW_CALL,	
	SHOW_TIME,	
};


typedef   struct{

    uint8_t         _head;	//报文头
    uint8_t      	_func;	//功能码
    uint8_t         _data1;	//
    uint8_t         _data2;	//
}MONITOR_T, *PMONITOR_T;

typedef   struct{

    int         _RX_ID;	
    int      	_TX_ID;
	int 		_fd;
    int         _showcall;
    int         _showtime;
    uint8_t     _flag;
}MONITORDATA_T, *PMONITORDATA_T;
extern MONITORDATA_T monitorData;

void	MonitorRx(int DevID, int CmpDevID, int fd, uint8_t *buffer, uint32_t len);
void 	MonitorTx(int DevID, int RealDevID, int fd, uint8_t *buffer, uint32_t len);
void 	Monitor_Enable(int DevID);
void 	Monitor_Disable(int DevID);
int 	Init_Monitor(int DevID);


#endif
