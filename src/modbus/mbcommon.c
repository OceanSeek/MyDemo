/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：mbcommon.c                                                     **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于实现Modbus各种情况下的公用部分                             **/
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2015-07-18          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/ 
#include "sys.h"
#include "mbcommon.h"

/*将接收到的写单个Coil值转化为布尔量，对应0x05功能码*/
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

/*检验所写数据是否符合物理量要求范围并处理(单精度浮点数)*/
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

/*检验所写数据是否符合物理量要求范围并处理(双精度浮点数)*/
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

/*检验所写数据是否符合物理量要求范围并处理(16位整数)*/
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

/*检验所写数据是否符合物理量要求范围并处理(32位整数)*/
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
		statusDataSrc[i] = gpDevice[DevNo].ModbusData.pCoilStatus[nPoint];
		log("qunatity is %d npoint is %d value is %d\n",quantity,nPoint,statusDataSrc[i]);
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
		RegDataSrc[i] = gpDevice[DevNo].ModbusData.pHoldingRegister[nPoint];
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




/*获取想要读取的Coil量的值*/
void GetCoilStatus(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusList)
{
  ReadCoilStatusData(startAddress, quantity, statusList);
}

/*获取想要读取的InputStatus量的值*/
void GetInputStatus(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusValue)
{
  //如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
	ReadInputStatusData(startAddress, quantity, statusValue);
}

/*获取想要读取的保持寄存器的值*/
void GetHoldingRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
 	//如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
 	ReadHoldingRegData(startAddress, quantity, registerValue);
}

/*获取想要读取的输入寄存器的值*/
void GetInputRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
 	//如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
	ReadInputgRegData(startAddress, quantity, registerValue);
}

/*设置单个线圈的值*/
void SetSingleCoil(int DevNo, uint16_t coilAddress,bool coilValue)
{
  //如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
}

/*设置单个寄存器的值*/
void SetSingleRegister(int DevNo, uint16_t registerAddress,uint16_t registerValue)
{
  //如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
}

/*设置多个线圈的值*/
void SetMultipleCoil(int DevNo, uint16_t startAddress,uint16_t quantity,bool *statusValue)
{
  //如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
}

/*设置多个寄存器的值*/
void SetMultipleRegister(int DevNo, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
  //如果需要Modbus TCP Server/RTU Slave应用中实现具体内容
}

/*更新读回来的线圈状态*/
void UpdateCoilStatus(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue)
{
  //在客户端（主站）应用中实现
	int startRegister = -1, i;

	for( i = 0; i < gpDevice[DevNo].ModbusData._CoilStatusNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pCoiStatus_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister = -1){
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

/*更新读回来的输入状态值*/
void UpdateInputStatus(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue)
{
  //在客户端（主站）应用中实现
	int startRegister = -1, i;

	for( i = 0; i < gpDevice[DevNo].ModbusData._InputStatusNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pInputStatus_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pInputStatus_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister = -1){
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

/*更新读回来的保持寄存器,功能码03*/
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
	if(startRegister = -1){
		perror("startRegister = -1");
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
	log("startRegister(%d) quantity(%d)\n", startAddress, quantity);
	for(i=0;i<quantity;i++)
	{
		log("data[%d] ",gpDevice[DevNo].ModbusData.pHoldingRegister[startRegister + i]);
	}
	log("\n");

	
  //在客户端（主站）应用中实现
// uint16_t startRegister=HoldingResterEndAddress+1;
//
//  switch(salveAddress)
//  {
//  case BPQStationAddress:       //更新读取的变频器参数
//    {
//      startRegister=36;
//      break;
//    }
//  case PUMPStationAddress:      //更新蠕动泵
//    {
////      aPara.phyPara.pumpRotateSpeed=registerValue[1];
//      startRegister=HoldingResterEndAddress+1;
//      break;
//    }
//  case JIG1StationAddress:      //更新摆臂小电机
//    {
//      startRegister=48;
//      break;
//    }
//  case JIG2StationAddress:      //更新摆臂小电机
//    {
//      startRegister=52;
//      break;
//    }
//  case JIG3StationAddress:      //更新摆臂小电机
//    {
//      startRegister=56;
//      break;
//    }
//  case HLPStationAddress:       //更新红外温度
//    {
//      aPara.phyPara.hlpObjectTemperature=registerValue[0]/100.0;
//      startRegister=HoldingResterEndAddress+1;
//      break;
//    }
//  case ROL1StationAddress:      //更新摆臂控制
//    {
//      startRegister=quantity<3?60:62;
//      break;
//    }
//  case ROL2StationAddress:      //更新摆臂控制
//    {
//      startRegister=quantity<3?70:72;
//      break;
//    }
//  case ROL3StationAddress:      //更新摆臂控制
//    {
//      startRegister=quantity<3?80:82;
//      break;
//    }
//  case DRUMStationAddress:      //更新滚筒电机
//    {
//      startRegister=quantity<3?90:92;
//      break;
//    }
//  default:                      //故障态
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

/*更新读回来的输入寄存器*/
void UpdateInputResgister(int DevNo, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue)
{
  //在客户端（主站）应用中实现
	int startRegister = -1, i;
	for( i = 0; i < gpDevice[DevNo].ModbusData._HoldingRegNum ; i++){
		if(salveAddress == gpDevice[DevNo].ModbusData.pInputResgister_T[i]._SlaverAddr){
			if(startAddress == gpDevice[DevNo].ModbusData.pInputResgister_T[i]._RegAddr){
				startRegister = i;
				break;
			}
		}
	}
	if(startRegister = -1){
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
