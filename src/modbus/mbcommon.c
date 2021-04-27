/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbcommon.c                                                     **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus��������µĹ��ò���                             **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2015-07-18          ľ��              �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 
#include "sys.h"
#include "mbcommon.h"

/*�����յ���д����Coilֵת��Ϊ����������Ӧ0x05������*/
bool CovertSingleCommandCoilToBoolStatus(uint16_t coilValue,bool value)
{
  bool state=value;
  if(coilValue==0x0000)
  {
    state=false;
  }
  if(coilValue==0xFF00)
  {
    state=true;
  }
  return state;
}

/*������д�����Ƿ����������Ҫ��Χ������(�����ȸ�����)*/
float CheckWriteFloatDataIsValid(float value,float range,float zero)
{
  if(value>=range)
  {
    return range;
  }
  else if(value<=zero)
  {
    return zero;
  }
  else
  {
    return value;
  }
}

/*������д�����Ƿ����������Ҫ��Χ������(˫���ȸ�����)*/
double CheckWriteDoubleDataIsValid(double value,double range,double zero)
{
  if(value>=range)
  {
    return range;
  }
  else if(value<=zero)
  {
    return zero;
  }
  else
  {
    return value;
  }
}

/*������д�����Ƿ����������Ҫ��Χ������(16λ����)*/
uint16_t CheckWriteInt16DataIsValid(uint16_t value,uint16_t range,uint16_t zero)
{
  if(value>=range)
  {
    return range;
  }
  else if(value<=zero)
  {
    return zero;
  }
  else
  {
    return value;
  }
}

/*������д�����Ƿ����������Ҫ��Χ������(32λ����)*/
uint32_t CheckWriteInt32DataIsValid(uint32_t value,uint32_t range,uint32_t zero)
{
  if(value>=range)
  {
    return range;
  }
  else if(value<=zero)
  {
    return zero;
  }
  else
  {
    return value;
  }
}

void ReadCoilStatusData(uint16_t startAddress,uint16_t quantity,bool *statusList)
{
	uint16_t wRealID,nPoint,i,DevNo;
	bool statusDataSrc[255];

	memset(statusDataSrc,0,255*sizeof(bool));

//	log("startAddress:%d;quantity:%d;\n", startAddress, quantity);
	for(i=0;i<quantity;i++){
		if((startAddress + i) >= gVars.TransModbusCoidStatusTableNum)continue; 
		wRealID = TransModbusCoidStatusTable[startAddress + i].wRealID;
		nPoint = TransModbusCoidStatusTable[startAddress + i].nPoint;
//		log("wRealID:%d;nPoint:%d;\n", wRealID, nPoint);
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return;
		}

		if(strcmp("PLC", gpDevice[DevNo].Name) == 0){
			statusDataSrc[i] = gpDevice[DevNo].ModbusData.pCoilStatus[nPoint];
		}
		if(strcmp("WQ900", gpDevice[DevNo].Name) == 0){
			bool value;
			value = gpDevice[DevNo].pBurstBI[nPoint].bStatus == 1?true:false;
			statusDataSrc[i] = value;
		}
		
		// log("qunatity is %d npoint is %d value is %d\n",quantity,nPoint,statusDataSrc[i]);
	}
	
	memcpy(statusList, statusDataSrc, quantity);
}

void ReadInputStatusData(uint16_t startAddress,uint16_t quantity,bool *statusList)
{
	uint16_t wRealID,nPoint,i,DevNo;
	bool statusDataSrc[255];

	memset(statusDataSrc,0,255*sizeof(bool));
	
	for(i=0;i<quantity;i++){
		if((startAddress + i) >= gVars.TransModbusInputStatusTableNum)continue; 
		wRealID = TransModbusInputStatusTable[startAddress + i].wRealID;
		nPoint = TransModbusInputStatusTable[startAddress + i].nPoint;
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return;
		}
		statusDataSrc[i] = gpDevice[DevNo].ModbusData.pInputStatus[nPoint];
	}
	
	memcpy(statusList, statusDataSrc, quantity);
}

void ReadHoldingRegData(uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
	uint16_t wRealID,nPoint,i,DevNo;
	uint16_t RegDataSrc[255];

	memset(RegDataSrc,0,255*sizeof(uint16_t));
	
//	log("startAddress:%d;quantity:%d;\n", startAddress, quantity);
	for(i=0;i<quantity;i++){
		if((startAddress + i) >= gVars.TransModbusHoldingRegTableNum){
			continue; 
		}
		wRealID = TransModbusHoldingRegTable[startAddress + i].wRealID;
		nPoint = TransModbusHoldingRegTable[startAddress + i].nPoint;
//		log("wRealID:%d;nPoint:%d;\n", wRealID, nPoint);
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return;
		}

		if(strcmp("PLC", gpDevice[DevNo].Name) == 0){
			RegDataSrc[i] = gpDevice[DevNo].ModbusData.pHoldingRegister[nPoint];
		}
		if(strcmp("WQ900", gpDevice[DevNo].Name) == 0){
			log("bType is [%d]\n", gpDevice[DevNo].pBurstAI[nPoint].bType);
			if(gpDevice[DevNo].pBurstAI[nPoint].bType == 2){
				RegDataSrc[i] = gpDevice[DevNo].pBurstAI[nPoint].detect16._detect;
			}
			else if(gpDevice[DevNo].pBurstAI[nPoint].bType == 4){
				
			}
		}
	}
	
	memcpy(registerValue, RegDataSrc, quantity*sizeof(uint16_t));
}

void ReadInputgRegData(uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
	uint16_t wRealID,nPoint,i,DevNo;
	uint16_t RegDataSrc[255];
	
	memset(RegDataSrc,0,255*sizeof(uint16_t));
	for(i=0;i<quantity;i++){
		if((startAddress + i) >= gVars.TransModbusInputRegTableNum)return; 
		wRealID = TransModbusInputRegTable[startAddress + i].wRealID;
		nPoint = TransModbusInputRegTable[startAddress + i].nPoint;
		
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return;
		}
		RegDataSrc[i] = gpDevice[DevNo].ModbusData.pInputResgister[nPoint];
	}
	
	memcpy(registerValue, RegDataSrc, quantity*sizeof(uint16_t));
}




/*��ȡ��Ҫ��ȡ��Coil����ֵ*/
void GetCoilStatus(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusList)
{
  ReadCoilStatusData(startAddress, quantity, statusList);
}

/*��ȡ��Ҫ��ȡ��InputStatus����ֵ*/
void GetInputStatus(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusValue)
{
  //�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
	ReadInputStatusData(startAddress, quantity, statusValue);
}

/*��ȡ��Ҫ��ȡ�ı��ּĴ�����ֵ*/
void GetHoldingRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
 	//�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
 	ReadHoldingRegData(startAddress, quantity, registerValue);
}

/*��ȡ��Ҫ��ȡ������Ĵ�����ֵ*/
void GetInputRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
 	//�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
	ReadInputgRegData(startAddress, quantity, registerValue);
}

/*���õ�����Ȧ��ֵ*/
void SetSingleCoil(int DevNo, uint16_t coilAddress,bool coilValue)
{
  //�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
}

/*���õ����Ĵ�����ֵ*/
void SetSingleRegister(int DevNo, uint16_t registerAddress,uint16_t registerValue)
{
  //�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
}

/*���ö����Ȧ��ֵ*/
void SetMultipleCoil(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusValue)
{
  //�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
}

/*���ö���Ĵ�����ֵ*/
void SetMultipleRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
  //�����ҪModbus TCP Server/RTU SlaveӦ����ʵ�־�������
}

/*���¶���������Ȧ״̬*/
void UpdateCoilStatus(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue)
{
  //�ڿͻ��ˣ���վ��Ӧ����ʵ��
	int startRegister = -1, i;

	for( i = 0; i < gpDevice[DevNo].ModbusData._CoilStatusNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister == -1){
		perror("startRegister = -1");
		return;
	}
	if(gpDevice[DevNo].ModbusData.pCoilStatus == NULL) return;

	if((startRegister + quantity) >= gpDevice[DevNo].ModbusData._CoilStatusNum){
		perror("startRegister + quantity > gpDevice[%d].ModbusData._CoilStatusNum",DevNo);
		return;
	}

	memcpy(&gpDevice[DevNo].ModbusData.pCoilStatus[startRegister], stateValue, quantity*sizeof(bool));
	log("startRegister(%d) quantity(%d)\n", startAddress, quantity);
	for(i=0;i<quantity;i++)
	{
		log("data[%d] ",gpDevice[DevNo].ModbusData.pCoilStatus[startRegister + i]);
		
	}
	log("\n");
  
}

/*���¶�����������״ֵ̬*/
void UpdateInputStatus(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue)
{
  //�ڿͻ��ˣ���վ��Ӧ����ʵ��
	int startRegister = -1, i;

	for( i = 0; i < gpDevice[DevNo].ModbusData._InputStatusNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pInputStatus_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pInputStatus_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister == -1){
		perror("startRegister = -1");
		return;
	}
	
	if(gpDevice[DevNo].ModbusData.pInputStatus == NULL) return;

	if(startRegister >= gpDevice[DevNo].ModbusData._InputStatusNum){
		perror("startRegister > gpDevice[%d].ModbusData._InputStatusNum",DevNo);
		return;
	}
	if((startRegister + quantity) >= gpDevice[DevNo].ModbusData._InputStatusNum){
		perror("startRegister + quantity > gpDevice[%d].ModbusData._InputStatusNum",DevNo);
		return;
	}

	memcpy(&gpDevice[DevNo].ModbusData.pInputStatus[startRegister], stateValue, quantity*sizeof(bool));
	log("startRegister(%d) quantity(%d)\n", startAddress, quantity);
	for(i=0;i<quantity;i++)
	{
		log("data[%d] ",gpDevice[DevNo].ModbusData.pInputStatus[startRegister + i]);
	}
	log("\n");
}

void UpdateHoldingRegister(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
	int startRegister = -1, i;
	for( i = 0; i < gpDevice[DevNo].ModbusData._HoldingRegNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pHoldingRegister_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
		
	}
	if(startRegister == -1){
		perror("startRegister = -1, startAddress is %d", startAddress);
		return;
	}
	if(gpDevice[DevNo].ModbusData.pHoldingRegister == NULL) return;
	
	if(startRegister >= gpDevice[DevNo].ModbusData._HoldingRegNum){
		perror("startRegister > gpDevice[%d].ModbusData._HoldingRegNum",DevNo);
		return;
	}
	if((startRegister + quantity) >= gpDevice[DevNo].ModbusData._HoldingRegNum){
		perror("startRegister + quantity > gpDevice[%d].ModbusData._HoldingRegNum",DevNo);
		return;
	}
	
	memcpy(&gpDevice[DevNo].ModbusData.pHoldingRegister[startRegister], registerValue, quantity*sizeof(uint16_t));
	log("startRegister(%d) quantity(%d)\n", startRegister, quantity);
	for(i=0;i<quantity;i++)
	{
		log("data[%d] ",gpDevice[DevNo].ModbusData.pHoldingRegister[startRegister + i]);
	}
	log("\n");

	
  //�ڿͻ��ˣ���վ��Ӧ����ʵ��
// uint16_t startRegister=HoldingResterEndAddress+1;
//
//  switch(salveAddress)
//  {
//  case BPQStationAddress:       //���¶�ȡ�ı�Ƶ������
//    {
//      startRegister=36;
//      break;
//    }
//  case PUMPStationAddress:      //�����䶯��
//    {
////      aPara.phyPara.pumpRotateSpeed=registerValue[1];
//      startRegister=HoldingResterEndAddress+1;
//      break;
//    }
//  case JIG1StationAddress:      //���°ڱ�С���
//    {
//      startRegister=48;
//      break;
//    }
//  case JIG2StationAddress:      //���°ڱ�С���
//    {
//      startRegister=52;
//      break;
//    }
//  case JIG3StationAddress:      //���°ڱ�С���
//    {
//      startRegister=56;
//      break;
//    }
//  case HLPStationAddress:       //���º����¶�
//    {
//      aPara.phyPara.hlpObjectTemperature=registerValue[0]/100.0;
//      startRegister=HoldingResterEndAddress+1;
//      break;
//    }
//  case ROL1StationAddress:      //���°ڱۿ���
//    {
//      startRegister=quantity<3?60:62;
//      break;
//    }
//  case ROL2StationAddress:      //���°ڱۿ���
//    {
//      startRegister=quantity<3?70:72;
//      break;
//    }
//  case ROL3StationAddress:      //���°ڱۿ���
//    {
//      startRegister=quantity<3?80:82;
//      break;
//    }
//  case DRUMStationAddress:      //���¹�Ͳ���
//    {
//      startRegister=quantity<3?90:92;
//      break;
//    }
//  default:                      //����̬
//    {
//      startRegister=HoldingResterEndAddress+1;
//      break;
//    }
//  }
//
//  if(startRegister<=HoldingResterEndAddress)
//  {
//    for(int i=0;i<quantity;i++)
//    {
//      aPara.holdingRegister[startRegister+i]=registerValue[i];
//    }
//  }  
}

/*���¶�����������Ĵ���*/
void UpdateInputResgister(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
  //�ڿͻ��ˣ���վ��Ӧ����ʵ��
	int startRegister = -1, i;
	for( i = 0; i < gpDevice[DevNo].ModbusData._HoldingRegNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pInputResgister_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pInputResgister_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister == -1){
		perror("startRegister = -1");
		return;
	}
	if(gpDevice[DevNo].ModbusData.pInputResgister == NULL) return;

	if(startRegister >= gpDevice[DevNo].ModbusData._InputRegNum){
		perror("startRegister > gpDevice[%d].ModbusData._InputRegNum",DevNo);
		return;
	}
	if((startRegister + quantity) >= gpDevice[DevNo].ModbusData._InputRegNum){
		perror("startRegister + quantity > gpDevice[%d].ModbusData._InputRegNum",DevNo);
		return;
	}

	memcpy(&gpDevice[DevNo].ModbusData.pInputResgister[startRegister], registerValue, quantity*sizeof(uint16_t));
	log("startRegister(%d) quantity(%d)\n", startRegister, quantity);
	for(i=0;i<quantity;i++)
	{
		log("data[%d] ",gpDevice[DevNo].ModbusData.pInputResgister[startRegister + i]);
	}
	log("\n");
  
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
