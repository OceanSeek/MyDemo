#include "sys.h"
#include "monitor.h"

MONITORDATA_T monitorData = {IDDISABLE, IDDISABLE, SHOWENABLE, SHOWENABLE, MONITOR_ENABLE};

int DealProtocl(int DevID, uint8_t *buf, uint16_t len);


void Monitor_Enable(int DevID)
{
	if(strcmp("Monitor", gpDevice[DevID].Protocol) == 0){
		monitorData._fd = gpDevice[DevID].fd;
		monitorData._flag = MONITOR_ENABLE;
		monitorData._RX_ID = IDDISABLE;
		monitorData._TX_ID = IDDISABLE;
	}
}

void Monitor_Disable(int DevID)
{
	if(strcmp("Monitor", gpDevice[DevID].Protocol) == 0){
		monitorData._flag = MONITOR_DISABLE;
	}

}

int ReciverMonitor(int DevID, uint8_t *buf, uint16_t len)
{
	if(monitorData._flag == MONITOR_DISABLE)
		return 0;
	
	if(buf[0] == 0x68 && len > 2)
		DealProtocl(DevID, buf, len);
	return 0;
}

int DealProtocl(int DevID, uint8_t *buf, uint16_t len){
	PMONITOR_T pMonitor;
	pMonitor = (PMONITOR_T)buf;

	monitorData._fd = gpDevice[DevID].fd;
	switch(pMonitor->_func){
		case SET_RX_TX_ID:
			monitorData._RX_ID = pMonitor->_data1;
			monitorData._TX_ID = pMonitor->_data2;
			break;
			break;
		case SHOW_CALL:
			monitorData._showcall = pMonitor->_data1;
			break;
		case SHOW_TIME:
			monitorData._showtime = pMonitor->_data1;
			break;
		default:
			break;
	}

}
/*******************************************************************  
*名称：      		MonitorRx  
*功能：			监控指定DevID号设备，并通过网口打印出设备DevID号的接收报文
*入口参数：         
*	@DevID 		设置想要监控设备的ID号
*	@RealDevID	实际设备的ID号
*	@fd			监控口的文件描述符
*	@buffer 	数据缓冲区
*	@len		数据长度
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
void MonitorRx(int DevID, int RealDevID, int fd, uint8_t *buffer, uint32_t len){

    uint32_t i;
	uint8_t buftmp[MAXBUFLEN];
	uint8_t *pbuff;
	int buflen;

	if(DevID != RealDevID)
		return;

	if(DevID == IDDISABLE)
		return;
	

	if(monitorData._flag == MONITOR_DISABLE)
		return;

//	monitorData._showcall = SHOWIDSABLE;
//	monitorData._showtime = SHOWIDSABLE;
	if(monitorData._showcall == SHOWIDSABLE){
		if(buffer[8] == 0x14 && buffer[6] == 0x01){
			return;
		}
		if(buffer[8] == 0x14 && buffer[6] == 0x0d){
			return;
		}
		if(buffer[6] == 0x64){
			return;
		}
		
	}
	if(monitorData._showtime == SHOWIDSABLE){
		if(buffer[6] == 0x67){
			return;
		}
	}
	
	pbuff = buftmp;
	sprintf(pbuff, "--->DevID(%d)protocol(%s)RX:  ",DevID, gpDevice[DevID].Protocol);
	buflen = strlen(pbuff);
	pbuff += buflen;
	
    for(i=0; i<len; i++){
		sprintf(pbuff, "%02x ",buffer[i]);
		pbuff += 3;
		if(pbuff - buftmp >= MAXBUFLEN){
			break;
		}
    }
	sprintf(pbuff, "\n");
	pbuff++;

	buflen = len*3 + buflen;
	
	if(buflen >= MAXBUFLEN){
		buflen = MAXBUFLEN;
	}
	write(fd, buftmp, buflen);
}
/*******************************************************************  
*名称：      		MonitorTx  
*功能：			监控指定DevID号设备，并通过网口打印出设备DevID号的发送报文
*入口参数：         
*	@DevID 		设置想要监控设备的ID号
*	@RealDevID	实际设备的ID号
*	@fd			监控口的文件描述符
*	@buffer 	数据缓冲区
*	@len		数据长度
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
void MonitorTx(int DevID, int RealDevID, int fd, uint8_t *buffer, uint32_t len){

    uint32_t i;
	uint8_t buftmp[MAXBUFLEN];
	uint8_t *pbuff;
	int buflen;

	if(DevID != RealDevID)
		return;
	
	if(DevID == IDDISABLE)
		return;
	

	if(monitorData._flag == MONITOR_DISABLE)
		return;
//	monitorData._showcall = SHOWIDSABLE;
//	monitorData._showtime = SHOWIDSABLE;
	if(monitorData._showcall == SHOWIDSABLE){
		if(buffer[8] == 0x14 && buffer[6] == 0x01){
			return;
		}
		if(buffer[8] == 0x14 && buffer[6] == 0x0d){
			return;
		}
		if(buffer[6] == 0x64){
			return;
		}
		
	}
	if(monitorData._showtime == SHOWIDSABLE){
		if(buffer[6] == 0x67){
			return;
		}
	}
	
	pbuff = buftmp;
	sprintf(pbuff, "<---DevID(%d)protocol(%s)TX:  ",DevID, gpDevice[DevID].Protocol);
	buflen = strlen(pbuff);
	pbuff += buflen;
	
    for(i=0; i<len; i++){
		sprintf(pbuff, "%02x ",buffer[i]);
		pbuff += 3;
		if(pbuff - buftmp >= MAXBUFLEN){
			break;
		}
    }
	sprintf(pbuff, "\n");
	pbuff++;

	
	buflen = len*3 + buflen;
	
	if(buflen >= MAXBUFLEN){
		buflen = MAXBUFLEN;
	}
	write(fd, buftmp, buflen);
}

int MonitorOnTimeOut(int DevID)
{
	
	
}
int Monitor_Task(int DevID)
{

}

int Init_Monitor(int DevID)
{
	gpDevice[DevID].Receive = ReciverMonitor;
	gpDevice[DevID].Task = Monitor_Task;
	gpDevice[DevID].OnTimeOut = MonitorOnTimeOut;
}

 
