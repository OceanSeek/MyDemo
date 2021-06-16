/*******************************************************************
Copyright (C):    
File name    :    ModbusTcpMaster.c
DESCRIPTION  :
AUTHOR       :	  WeiTao
Version      :    1.0
Date         :    2020/11/13
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/

#include "sys.h"



int ModbusTcpMaster_Deal(int DevNo, uint8_t *pbuf, uint16_t len);
int ModbusTcpMaster_Receive(int DevNo, uint8_t *buf, uint16_t len);
int ModbusTcpMaster_OnTimeOut(int DevNo);
int ModbusTcpMaster_Task(int DevNo);

TCPAccessedServerType mbServer;
TCPLocalClientType mbClient;  

uint8_t readCommand[10][12];
WritedCoilListNode coilList[3]={{0,0,0,1},{1,0,0,1},{2,0,0,0}};
WritedRegisterListNode registerList[3]={{0,0,0,1,1},{1,0,0,1,2},{2,0,0,0,0}};
union {
    uint32_t ipNumber;
    uint8_t ipSegment[4];
}ipAddress;

int Init_ModbusTcpMaster(int DevNo)
{
	gpDevice[DevNo].Receive = ModbusTcpMaster_Receive;
	gpDevice[DevNo].Task = ModbusTcpMaster_Task;
	gpDevice[DevNo].OnTimeOut = ModbusTcpMaster_OnTimeOut;
	gpDevice[DevNo].pModbusTcpMaster = (TCPLocalClientType*)malloc(sizeof(TCPLocalClientType));
	gpDevice[DevNo].pSendBuf = malloc(300);
	/*初始化TCP客户端对象*/
	InitializeTCPClientObject(gpDevice[DevNo].pModbusTcpMaster, NULL, NULL, NULL, NULL);
	ipAddress.ipNumber = ip2long(gpDevice[DevNo].IP);
	/* 实例化TCP服务器对象 */
	InstantiateTCPServerObject(&mbServer,          //要实例化的服务器对象
			gpDevice[DevNo].pModbusTcpMaster,             //服务器所属本地客户端对象
			ipAddress.ipSegment[0],                     //IP地址第1段
			ipAddress.ipSegment[1],                     //IP地址第2段
			ipAddress.ipSegment[2],                     //IP地址第3段
			ipAddress.ipSegment[3],                     //IP地址第4段
			gpDevice[DevNo].PORT,	//端口号
			1,						//读命令数量
			readCommand,					//读命令列表
			0,              //可写线圈量节点的数量
			NULL,          //写线圈列表
			0,          //可写寄存器量节点的数量
			NULL);  //写寄存器列表



//	log("_CoilStatusNum:%d,_InputRegNum:%d,_HoldingRegNum:%d,_InputRegNum%d\n"\
//		,gpDevice[DevNo].ModbusData._CoilStatusNum\
//		,gpDevice[DevNo].ModbusData._InputStatusNum\
//		,gpDevice[DevNo].ModbusData._HoldingRegNum\
//		,gpDevice[DevNo].ModbusData._InputRegNum);
	

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
	
	pFun = pbuf[7];
	num = pbuf[8];
	switch(pFun)
	{
		case 1:		//����Ȧ	
		case 2:		//����ɢ������
		case 3:		//�����ּĴ���
		case 4:		//������Ĵ���
			buflen = num + 5 + 4; 
			return buflen;
		default:
			// perror("function code(0x:%02x) > 0x04 \n", pFun);
			return RET_ERROR;
	}

}


uint8_t ModbusTcpMaster_Send(int DevNo,char *buf, int len){

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
		// DumpHEX(buf, len);
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
			return RET_ERROR;
		}
		usleep(10000);
		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
    }
	else if(strcmp("Net", gpDevice[DevNo].Com) == 0){
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			return RET_ERROR;
		}
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


int ModbusTcpMaster_Receive(int DevNo, uint8_t *buf, uint16_t len)
{

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
	uint8_t Min_BUFF_Len = 3;//最短报文长度
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
				DumpHEX(buf, len);
                perror("LenRemain error(%d) \n",LenRemain);
                return RET_ERROR;
            }
            ModbusTcpMaster_Deal(DevNo, BufTemp, LenTmp);
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}

int ModbusTcpMaster_Deal(int DevNo, uint8_t *pbuf, uint16_t len)
{
	if(gpDevice[DevNo].pModbusTcpMaster->pServerList == NULL){
		log("gpDevice[DevNo].pModbusTcpMaster->pServerList == NULL\n");
		return 0;
	}

	ParsingServerRespondMessage(DevNo, gpDevice[DevNo].pModbusTcpMaster, pbuf);
}


void ModbusTcp_SetAskCoilStatusFlag(int DevNo)
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

void ModbusTcp_SetAskInputStatusFlag(int DevNo)
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

void ModbusTcp_SetAskHoldingRegisterFlag(int DevNo)
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

void ModbusTcp_SetAskInputResgisterFlag(int DevNo)
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

void ModbusTcp_SetAskFlag(int DevNo)
{
	ModbusTcp_SetAskCoilStatusFlag(DevNo);
	ModbusTcp_SetAskInputStatusFlag(DevNo);
	ModbusTcp_SetAskHoldingRegisterFlag(DevNo);
	ModbusTcp_SetAskInputResgisterFlag(DevNo);
}

int ModbusTcpMaster_ASK(int DevNo, uint16_t SlaverAddr, int functionCode, int addrStart, int quantity)
{
	uint16_t len;
	ObjAccessInfo slaveInfo;
	
	slaveInfo.functionCode = functionCode;
	slaveInfo.quantity = quantity;
	slaveInfo.startingAddress = addrStart;
	slaveInfo.unitID = SlaverAddr;
	len = SyntheticReadWriteTCPServerCommand(DevNo, slaveInfo, NULL, NULL, gpDevice[DevNo].pSendBuf);
	// log("send data:");
	// DumpHEX(gpDevice[DevNo].pSendBuf, len);
	ModbusTcpMaster_Send(DevNo, gpDevice[DevNo].pSendBuf, len);
	//读命令存储下来，与接下来的接收数据比对
	uint8_t *pDest;
	pDest = gpDevice[DevNo].pModbusTcpMaster->pServerList->pReadCommand[0];
	memcpy(pDest, gpDevice[DevNo].pSendBuf, 12);
}

int ModbusTcpMaster_Write(int DevNo, uint16_t SlaverAddr, int functionCode, uint16_t *ResgisterList, int addrStart, int quantity)
{
	uint16_t len;
	ObjAccessInfo slaveInfo;
	
	slaveInfo.functionCode = functionCode;
	slaveInfo.quantity = quantity;
	slaveInfo.startingAddress = addrStart;
	slaveInfo.unitID = SlaverAddr;
	len = SyntheticReadWriteTCPServerCommand(DevNo, slaveInfo, NULL, ResgisterList, gpDevice[DevNo].pSendBuf);
	// log("send data:");
	// DumpHEX(gpDevice[DevNo].pSendBuf, len);
	ModbusTcpMaster_Send(DevNo, gpDevice[DevNo].pSendBuf, len);
	//读命令存储下来，与接下来的接收数据比对
	uint8_t *pDest;
	pDest = gpDevice[DevNo].pModbusTcpMaster->pServerList->pReadCommand[0];
	memcpy(pDest, gpDevice[DevNo].pSendBuf, 12);
}




int ModbusTcp_AskCoilStatus(int DevNo)
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
			if(quantity > Modbus_Ask_Max_Num) break;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusTcpMaster_ASK(DevNo, OldSlaverAddr, ReadCoilStatus, startAddr, quantity);
	return true;
}

int ModbusTcp_AskInputStatus(int DevNo)
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
			if(quantity > Modbus_Ask_Max_Num) break;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskInputStatus[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusTcpMaster_ASK(DevNo, OldSlaverAddr, ReadInputStatus, startAddr, quantity);
	return true;
}

int ModbusTcp_AskHoldingRegister(int DevNo)
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
			if(quantity > Modbus_Ask_Max_Num) break;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
	log("\nModbusTcpMaster Ask Info:DevID is %d addrStart is %d quantity is %d  SlaverAddr is %d\n", gpDevice[DevNo].ID, startAddr, quantity, OldSlaverAddr);
	
	ModbusTcpMaster_ASK(DevNo, OldSlaverAddr, ReadHoldingRegister, startAddr, quantity);
	return true;

}

int ModbusTcp_AskInputResgister(int DevNo)
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
			if(quantity > Modbus_Ask_Max_Num) break;
			addr[quantity] = NewStartAddr;
			quantity++;
			gpDevice[DevNo].ModbusData.pFlag_AskInputResgister[i] = false;
		}
	}
	if(quantity == 0) return false;
	startAddr = addr[0];
//	log("DevID is %d addrStart is %d quantity is %d\n", gpDevice[DevNo].ID, startAddr, quantity);
	ModbusTcpMaster_ASK(DevNo, OldSlaverAddr, ReadInputRegister, startAddr, quantity);
	return true;
}

int ModbusTcp_WriteSingleHoldingResgister(int DevNo)
{
	int i;
	uint16_t value, SlaverAddr, StartRegAddr;
	uint16_t ResgisterList[1];
	for(i = 0 ; i < gpDevice[DevNo].ModbusData._HoldingRegNum; i++){
		if(gpDevice[DevNo].ModbusData.pFlag_WriteSingleHoldingRegister[i] == true){
			PRINT_FUNLINE;
			gpDevice[DevNo].ModbusData.pFlag_WriteSingleHoldingRegister[i] = false;
			value = gpDevice[DevNo].ModbusData.pWriteSingleHoldingRegisterValue[i];
			SlaverAddr = gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._SlaverAddr;
			StartRegAddr = gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._RegAddr;
			ResgisterList[0] = value;
			ModbusTcpMaster_Write(DevNo, SlaverAddr, WriteSingleRegister, &ResgisterList[0], StartRegAddr, 1);
			return true;
		}
	}
	return false;
}

int MModbusTcp_SendAsk(int DevNo)
{
	if(ModbusTcp_WriteSingleHoldingResgister(DevNo))
		return true;
	if(ModbusTcp_AskCoilStatus(DevNo))
		return true;
	if(ModbusTcp_AskInputStatus(DevNo))
		return true;
	if(ModbusTcp_AskHoldingRegister(DevNo))
		return true;
	if(ModbusTcp_AskInputResgister(DevNo))
		return true;
	
	ModbusTcp_SetAskFlag(DevNo);
	return true;
}

int ModbusTcpMaster_OnTimeOut(int DevNo)
{
	gpDevice[DevNo].TimeCnt++;
	
	if(gpDevice[DevNo].TimeCnt%20 == 0){
		// ModbusTcp_SetAskFlag(DevNo);
	}

	if(gpDevice[DevNo].TimeCnt%1 == 0){
		MModbusTcp_SendAsk(DevNo);
	}
	
	usleep(50000);
	return RET_SUCESS;
}

int ModbusTcpMaster_Task(int DevNo)
{
	return RET_SUCESS;
}

