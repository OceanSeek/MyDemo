/*******************************************************************
Copyright (C):    
File name    :    ModbusRtuMaster.c
DESCRIPTION  :
AUTHOR       :	  WeiTao
Version      :    1.0
Date         :    2020/11/13
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/

#include "sys.h"
//#include "ModbusRtuMaster.h"


int ModbusRtuMaster_Deal(int DevNo, uint8_t *pbuf, uint16_t len);
int ModbusRtuMaster_Receive(int DevNo, uint8_t *buf, uint16_t len);
int ModbusRtuMaster_OnTimeOut(int DevNo);
int ModbusRtuMaster_Task(int DevNo);

RTUAccessedSlaveType slaver_id1;//��վ��ʱ����Ϊֻ��һ��


int Init_ModbusRtuMaster(int DevNo)
{

	gpDevice[DevNo].Receive = ModbusRtuMaster_Receive;
	gpDevice[DevNo].Task = ModbusRtuMaster_Task;
	gpDevice[DevNo].OnTimeOut = ModbusRtuMaster_OnTimeOut;
	gpDevice[DevNo].pModbusMaster = (RTULocalMasterType*)malloc(sizeof(RTULocalMasterType));
	gpDevice[DevNo].pSendBuf = malloc(300);
	slaver_id1.stationAddress = 1;//��վ��ַ
	gpDevice[DevNo].pModbusMaster->slaveNumber = 1;
	gpDevice[DevNo].pModbusMaster->pSlave = &slaver_id1;
	InitializeRTUMasterObject(gpDevice[DevNo].pModbusMaster, 1, &slaver_id1, NULL, NULL, NULL, NULL);
//	log("_CoilStatusNum:%d,_InputRegNum:%d,_HoldingRegNum:%d,_InputRegNum%d\n"\
//		,gpDevice[DevNo].ModbusData._CoilStatusNum\
//		,gpDevice[DevNo].ModbusData._InputStatusNum\
//		,gpDevice[DevNo].ModbusData._HoldingRegNum\
//		,gpDevice[DevNo].ModbusData._InputRegNum);
	log("devno is %d\n",DevNo);

	/*test*/
//	int i = 0;
//	int HoldingRegNum = gpDevice[DevNo].ModbusData._HoldingRegNum;
//	int InputRegNum = gpDevice[DevNo].ModbusData._InputRegNum;
//	int CoilStatusNum = gpDevice[DevNo].ModbusData._CoilStatusNum;
//	
//	for(i = 0 ;i < HoldingRegNum;i++){
//		gpDevice[DevNo].ModbusData.pHoldingRegister[i] = gpDevice[DevNo].ID*100 + i;
//	}
//	for(i = 0 ;i < InputRegNum;i++){
//		gpDevice[DevNo].ModbusData.pInputResgister[i] = gpDevice[DevNo].ID*100 + i;
//	}
//	for(i = 0 ;i < CoilStatusNum;i++){
//		gpDevice[DevNo].ModbusData.pCoilStatus[i] = i%2;
//	}
//	PRINT_FUNLINE;
	/*endtest*/
	
}


//��鱨�ĺϷ���
static int SearchOneFrame(uint8_t *pbuf, uint16_t len){
	uint8_t Min_BUFF_Len = 7;//��̱��ĳ���
	uint8_t pFun;
	int buflen=0;
	uint8_t num = 0;
	
	if(pbuf == NULL)return RET_ERROR;
//	if(len<Min_BUFF_Len)log("len<Min_BUFF_Len\n");	
//	if(len<Min_BUFF_Len)return RET_ERROR;	
	
	pFun = pbuf[1];
	num = pbuf[2];
	switch(pFun)
	{
		case 1:		//����Ȧ	
		case 2:		//����ɢ������
		case 3:		//�����ּĴ���
		case 4:		//������Ĵ���
			buflen = num + 5; 
			return buflen;
		case 5: 	//д����Ȧ
		case 6: 	//д�����Ĵ���
			return buflen = 8;
			break;
		default:
			return RET_ERROR;
	}

}


uint8_t ModbusRtuMaster_Send(int DevNo,char *buf, int len){

	// if(strcmp("UDP", gpDevice[DevNo].TcpType) == 0){
	// 	struct sockaddr_in dest_addr = {};
	// 	dest_addr.sin_family = AF_INET;//ipv4
	// 	dest_addr.sin_port = htons(gpDevice[DevNo].UDP_Dest_PORT);
	// 	dest_addr.sin_addr.s_addr = inet_addr(gpDevice[DevNo].UDP_Dest_IP);
	// 	sendto(gpDevice[DevNo].fd, buf, len, 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
	// }
    // else {
	// 	log("devno is %d, fd is %d\n", DevNo, gpDevice[DevNo].fd);
	// 	if(-1 == write(gpDevice[DevNo].fd, buf,len)){
	// 		perror("Send error fd ,fd is (%d)\n", gpDevice[DevNo].fd);
	// 		return RET_ERROR;
    // 	}
	// }

#ifdef HUAWEI
	if(strcmp("Serial", gpDevice[DevNo].Com) == 0){
		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Write);
		DumpHEX(buf, len);
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
			return RET_ERROR;
		}
		usleep(10000);
		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
    }
#else
	if(strcmp("Serial", gpDevice[DevNo].Com) == 0){
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			return RET_ERROR;
		}		
    }
#endif

		
    gpDevice[DevNo].ReSendNum++;

	MonitorTx(monitorData._TX_ID, DevNo, monitorData._fd, buf, len);
    return RET_SUCESS;
}


int ModbusRtuMaster_Receive(int DevNo, uint8_t *buf, uint16_t len)
{

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
	uint8_t Min_BUFF_Len = 6;//最短报文长度
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
//		log("ret len is %d\n",ret);
        if(ret != RET_ERROR){
            LenTmp = ret;
            if(LenRemain < Min_BUFF_Len){
                perror("LenRemain error(%d) \n",LenRemain);
                return RET_ERROR;
            }
            ModbusRtuMaster_Deal(DevNo, BufTemp, LenTmp);
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}

int ModbusRtuMaster_Deal(int DevNo, uint8_t *pbuf, uint16_t len)
{
	uint8_t pFun = 0;
	uint16_t num = 0;
	bool ret;

	ret = CheckRTUMessageIntegrity(pbuf,len);
	if(!ret)
	{
		perror("crc16 error\n");
		return RET_ERROR;
	}
	pFun = pbuf[1];
	num = MAKEWORD(pbuf[4], pbuf[5]);
	ParsingSlaveRespondMessage(DevNo, gpDevice[DevNo].pModbusMaster, pbuf, gpDevice[DevNo].pSendBuf);
	
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
			break;
		case 16:	//д����Ĵ���
			break;
		default:
			return RET_ERROR;
	}


}


void SetAskCoilStatusFlag(int DevNo)
{
	int i,nPoint;
	
	for( i = 0; i < gVars.TransModbusCoidStatusTableNum; i++){
		if(TransModbusCoidStatusTable[i].wRealID == gpDevice[DevNo].ID){
			nPoint = TransModbusCoidStatusTable[i].nPoint;
			if( nPoint >= gpDevice[DevNo].ModbusData._CoilStatusNum) continue;
			gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus[nPoint] = true;
		}
	}
}

void SetAskInputStatusFlag(int DevNo)
{
	int i,nPoint;
	
	for( i = 0; i < gVars.TransModbusInputStatusTableNum; i++){
		if(TransModbusInputStatusTable[i].wRealID == gpDevice[DevNo].ID){
			nPoint = TransModbusInputStatusTable[i].nPoint;
			if( nPoint >= gpDevice[DevNo].ModbusData._InputStatusNum) continue;
			gpDevice[DevNo].ModbusData.pFlag_AskInputStatus[nPoint] = true;
		}
	}
	
}

void SetAskHoldingRegisterFlag(int DevNo)
{
	int i,nPoint;
	
	for( i = 0; i < gVars.TransModbusHoldingRegTableNum; i++){
		if(TransModbusHoldingRegTable[i].wRealID == gpDevice[DevNo].ID){
			nPoint = TransModbusHoldingRegTable[i].nPoint;
			if( nPoint >= gpDevice[DevNo].ModbusData._HoldingRegNum) continue;
			gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister[nPoint] = true;
		}
	}
}

void SetAskInputResgisterFlag(int DevNo)
{
	int i,nPoint;
	
	for( i = 0; i < gVars.TransModbusInputRegTableNum; i++){
		if(TransModbusInputRegTable[i].wRealID == gpDevice[DevNo].ID){
			nPoint = TransModbusInputRegTable[i].nPoint;
			if( nPoint >= gpDevice[DevNo].ModbusData._InputRegNum) continue;
			gpDevice[DevNo].ModbusData.pFlag_AskInputResgister[nPoint] = true;
		}
	}
}

void SetAskFlag(int DevNo)
{
	SetAskCoilStatusFlag(DevNo);
	SetAskInputStatusFlag(DevNo);
	SetAskHoldingRegisterFlag(DevNo);
	SetAskInputResgisterFlag(DevNo);
}

int ModbusRtuMaster_ASK(int DevNo, uint16_t SlaverAddr, int functionCode, int addrStart, int quantity)
{
	uint16_t len;
	ObjAccessInfo slaveInfo;
	
	slaveInfo.functionCode = functionCode;
	slaveInfo.quantity = quantity;
	slaveInfo.startingAddress = addrStart;
	slaveInfo.unitID = SlaverAddr;
	len = SyntheticReadWriteSlaveCommand(DevNo, slaveInfo, NULL, NULL, gpDevice[DevNo].pSendBuf);
	
	ModbusRtuMaster_Send(DevNo, gpDevice[DevNo].pSendBuf, len);
}


int ModbusAskCoilStatus(int DevNo)
{	
	int i, quantity = 0, startAddr, oldStartAddr, NewStartAddr, NewSlaverAddr, OldSlaverAddr;
	int addr[128];
	for( i = 0; i < gpDevice[DevNo].ModbusData._CoilStatusNum; i++){
		if(gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus[i] == true){
			NewStartAddr = gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._RegAddr;
			NewSlaverAddr = gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._SlaverAddr;
			if(quantity > 0){
				if((NewStartAddr - oldStartAddr) != 1) break;
				if((NewSlaverAddr - OldSlaverAddr) != 0) break;
			}
			oldStartAddr = NewStartAddr;
			OldSlaverAddr = NewSlaverAddr;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusRtuMaster_ASK(DevNo, OldSlaverAddr, ReadCoilStatus, startAddr, quantity);
	return true;
}

int ModbusAskInputStatus(int DevNo)
{
	int i, quantity = 0, startAddr, oldStartAddr, NewStartAddr, NewSlaverAddr, OldSlaverAddr;
	int addr[128];
	for( i = 0; i < gpDevice[DevNo].ModbusData._InputStatusNum; i++){
		if(gpDevice[DevNo].ModbusData.pFlag_AskInputStatus[i] == true){
			NewStartAddr = gpDevice[DevNo].ModbusData.pInputStatus_T[i]._RegAddr;
			NewSlaverAddr = gpDevice[DevNo].ModbusData.pInputStatus_T[i]._SlaverAddr;
			if(quantity > 0){
				if((NewStartAddr - oldStartAddr) != 1) break;
				if((NewSlaverAddr - OldSlaverAddr) != 0) break;
			}
			oldStartAddr = NewStartAddr;
			OldSlaverAddr = NewSlaverAddr;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskInputStatus[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusRtuMaster_ASK(DevNo, OldSlaverAddr, ReadInputStatus, startAddr, quantity);
	return true;
}

int ModbusAskHoldingRegister(int DevNo)
{
	int i, quantity = 0, startAddr, oldAddr, NewStartAddr, NewSlaverAddr, OldSlaverAddr;
	int addr[128];
	for( i = 0; i < gpDevice[DevNo].ModbusData._HoldingRegNum; i++){
		if(gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister[i] == true){
			NewStartAddr = gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._RegAddr;
			NewSlaverAddr = gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._SlaverAddr;
			if(quantity > 0){
				if((NewStartAddr - oldAddr) != 1) break;
				if((NewSlaverAddr - OldSlaverAddr) != 0) break;
			}
			oldAddr = NewStartAddr;
			OldSlaverAddr = NewSlaverAddr;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
	if(DevNo == 0)
		log("\nDevID is %d addrStart is %d quantity is %d  OldSlaverAddr is %d", gpDevice[DevNo].ID, startAddr, quantity, OldSlaverAddr);
	ModbusRtuMaster_ASK(DevNo, OldSlaverAddr, ReadHoldingRegister, startAddr, quantity);
	return true;

}

int ModbusAskInputResgister(int DevNo)
{
	int i, quantity = 0, startAddr, oldAddr, NewStartAddr, NewSlaverAddr, OldSlaverAddr;
	int addr[128];
	for( i = 0; i < gpDevice[DevNo].ModbusData._InputRegNum; i++){
		if(gpDevice[DevNo].ModbusData.pFlag_AskInputResgister[i] == true){
			NewStartAddr = gpDevice[DevNo].ModbusData.pInputResgister_T[i]._RegAddr;
			NewSlaverAddr = gpDevice[DevNo].ModbusData.pInputResgister_T[i]._SlaverAddr;
			if(quantity > 0){
				if((NewStartAddr - oldAddr) != 1) break;
				if((NewSlaverAddr - OldSlaverAddr) != 0) break;
			}
			oldAddr = NewStartAddr;
			OldSlaverAddr = NewSlaverAddr;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskInputResgister[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusRtuMaster_ASK(DevNo, OldSlaverAddr, ReadInputRegister, startAddr, quantity);
	return true;
}

int ModbusSendAsk(int DevNo)
{
	
	// ModbusAskCoilStatus(DevNo);
	// ModbusAskInputStatus(DevNo);
	// ModbusAskHoldingRegister(DevNo);
	// ModbusAskInputResgister(DevNo);

	if(ModbusAskCoilStatus(DevNo))
		return true;
	if(ModbusAskInputStatus(DevNo))
		return true;
	if(ModbusAskHoldingRegister(DevNo))
		return true;
	if(ModbusAskInputResgister(DevNo))
		return true;
}

void Modbus_TestData(int DevNo, uint32_t timeCnt)
{
	int i;
	int a;
	for( i = 0 ; i < gpDevice[DevNo].ModbusData._HoldingRegNum; i++){
		srand(i+1+timeCnt);
		a = rand()%1000;
		gpDevice[DevNo].ModbusData.pHoldingRegister[i] = a;
//		log("i is %d a is %d\n", i, a);
	}

	for( i = 0 ; i < gpDevice[DevNo].ModbusData._CoilStatusNum; i++){
		srand(i+1+timeCnt);
		a = rand()%2;
		gpDevice[DevNo].ModbusData.pCoilStatus[i] = a;
	}

}

int ModbusRtuMaster_OnTimeOut(int DevNo)
{
	gpDevice[DevNo].TimeCnt++;
	
	if(gpDevice[DevNo].TimeCnt%20 == 0){
		SetAskFlag(DevNo);
	}

	if(gpDevice[DevNo].TimeCnt%1 == 0){
		ModbusSendAsk(DevNo);
	}

	if(gpDevice[DevNo].TimeCnt%5 == 0){
		Modbus_TestData(DevNo, gpDevice[DevNo].TimeCnt);
	}

	return RET_SUCESS;
}

int ModbusRtuMaster_Task(int DevNo)
{
	return RET_SUCESS;
}

