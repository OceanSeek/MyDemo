/*******************************************************************
Copyright (C):    
File name    :    ModbusRtuSlaver.c
DESCRIPTION  :
AUTHOR       :	  WeiTao
Version      :    1.0
Date         :    2020/11/13
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/

#include "sys.h"
//#include "ModbusRtuSlaver.h"



int Modbus_Deal(int DevNo, uint8_t *pbuf, uint16_t len);
int Modbus_Rtu_Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len);
int Modbus_Rtu_Slaver_OnTimeOut(int DevNo);
int ModbusRtuSlaver_Send(int DevNo,char *buf, int len);




int Init_ModbusRtuSlaver(int DevNo)
{

	gpDevice[DevNo].Receive = Modbus_Rtu_Slaver_Receive;
//	gpDevice[DevNo].Task = GX101Slaver_Task;
	gpDevice[DevNo].OnTimeOut = Modbus_Rtu_Slaver_OnTimeOut;

	gpDevice[DevNo].pSendBuf = malloc(400);

	gpDevice[DevNo].ModbusData._CoilStatusNum = 200;
	gpDevice[DevNo].ModbusData._InputStatusNum = 200;
	gpDevice[DevNo].ModbusData._HoldingRegNum = 200;
	gpDevice[DevNo].ModbusData._InputRegNum = 200;
	
	gpDevice[DevNo].ModbusData.pCoilStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._CoilStatusNum);
	gpDevice[DevNo].ModbusData.pInputStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._InputStatusNum);
	gpDevice[DevNo].ModbusData.pHoldingRegister = (uint16_t*)malloc(gpDevice[DevNo].ModbusData._HoldingRegNum);
	gpDevice[DevNo].ModbusData.pInputResgister = (uint16_t*)malloc(gpDevice[DevNo].ModbusData._InputRegNum);

	memset(gpDevice[DevNo].ModbusData.pCoilStatus, 0, gpDevice[DevNo].ModbusData._CoilStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pInputStatus, 0, gpDevice[DevNo].ModbusData._InputStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pHoldingRegister, 0, gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pInputResgister, 0, gpDevice[DevNo].ModbusData._InputRegNum*sizeof(bool));


}


int Modbus_Rtu_Slaver_OnTimeOut(int DevNo)
{

	gpDevice[DevNo].TimeCnt++;
	
	

}

int  ModbusRtuSlaver_Send(int DevNo,char *buf, int len)
{

	if(strcmp("UDP", gpDevice[DevNo].TcpType) == 0){
		struct sockaddr_in dest_addr = {};
		dest_addr.sin_family = AF_INET;//ipv4
		dest_addr.sin_port = htons(gpDevice[DevNo].UDP_Dest_PORT);
		dest_addr.sin_addr.s_addr = inet_addr(gpDevice[DevNo].UDP_Dest_IP);
		sendto(gpDevice[DevNo].fd, buf, len, 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
	}
    else if(-1 == write(gpDevice[DevNo].fd, buf,len)){
        perror("Send error fd ,fd is (%d)\n",gpDevice[DevNo].fd);
        return RET_ERROR;
    }
    gpDevice[DevNo].ReSendNum++;

	MonitorTx(monitorData._TX_ID, DevNo, monitorData._fd, buf, len);
    return RET_SUCESS;
}



//��鱨�ĺϷ���
static int SearchOneFrame(uint8_t *pbuf, uint16_t len){
	uint8_t Min_BUFF_Len = 6;//��̱��ĳ���
	uint8_t pFun;
	int buflen=0;
	int num = 0;
	
	if(pbuf == NULL)return RET_ERROR;
	if(len<Min_BUFF_Len)return RET_ERROR;	
	pFun = pbuf[1];
	num = pbuf[2];
	switch(pFun)
	{
		case 1:		//����Ȧ	
		case 2:		//����ɢ������
		case 3:		//�����ּĴ���
		case 4:		//������Ĵ���
		case 5: 	//д����Ȧ
		case 6: 	//д�����Ĵ���
			buflen = 8;
			return buflen;
			break;
		case 16:	//д����Ĵ���
			buflen = 2*num + 9;

			return buflen;
		default:
			return RET_ERROR;
	}

}


int Modbus_Rtu_Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len)
{

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
	uint8_t Min_BUFF_Len = 8;//��̱��ĳ���
	int16_t ret = -1;
	
    if(buf == NULL){
        perror("buffer (null)");
        return RET_ERROR;
    }
    if(len <= 0 || len>256){
		perror("buffer len error(%d)",len);
		log("DevNo(%d)  fd(%d)\n",DevNo, gpDevice[DevNo].fd);
        return RET_ERROR;
    }

    BufTemp = buf;
    LenRemain = len;
    while(BufTemp<buf+len){
		ret = SearchOneFrame(buf, len);
        if(ret != RET_ERROR){
            LenTmp = ret;
            if(LenRemain < Min_BUFF_Len){
                perror("len error(%d) \n",len);
                return RET_ERROR;
            }
			log("Dev %d:receive:", DevNo);
			DumpHEX(buf, len);
            Modbus_Deal(DevNo, BufTemp, LenTmp);
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}

int Modbus_Deal(int DevNo, uint8_t *pbuf, uint16_t len)
{
	uint8_t pFun = 0;
	uint16_t num = 0;
	bool ret;
	uint16_t SendLen;

	ret = CheckRTUMessageIntegrity(pbuf,len);
	if(!ret)
	{
		perror("crc16 error\n");
		return RET_ERROR;
	}

	pFun = pbuf[1];
	num = MAKEWORD(pbuf[4], pbuf[5]);
	SendLen = ParsingMasterAccessCommand(DevNo, pbuf, gpDevice[DevNo].pSendBuf, len, gpDevice[DevNo].Address);
	LogSysLocalTime();
	log("Dev %d:slaver send:",DevNo);
	DumpHEX(gpDevice[DevNo].pSendBuf, SendLen);
	ModbusRtuSlaver_Send(DevNo, gpDevice[DevNo].pSendBuf, SendLen);
	switch(pFun)
	{
		case 1:		//����Ȧ	
			break;
		case 2:		//����ɢ������
			break;
		case 3:		//�����ּĴ���
			break;
		case 4:		//������Ĵ���
			break;
		case 5: 	//д����Ȧ
			break;
		case 6: 	//д�����Ĵ���
			PRINT_FUNLINE;
			break;
		case 16:	//д����Ĵ���
			break;
		default:
			return RET_ERROR;
	}


}

