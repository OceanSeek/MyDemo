/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：modbusrtuslave.c                                               **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于声明Modbus RTU从站相关属性及方法                           **/
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2016-04-17          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/
#include "sys.h"
#include "mbrtuslave.h"

//#define StationAddress 0x01   /*定义本站地址*/

/*处理读线圈状态命令*/
static uint16_t HandleReadCoilStatusCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理读输入状态命令*/
static uint16_t HandleReadInputStatusCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理读保持寄存器命令*/
static uint16_t HandleReadHoldingRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理读输入寄存器命令*/
static uint16_t HandleReadInputRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理写单个线圈命令*/
static uint16_t HandleWriteSingleCoilCommand(int DevNo, uint16_t coilAddress, uint16_t coilValue, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理写单个寄存器命令*/
static uint16_t HandleWriteSingleRegisterCommand(int DevNo, uint16_t registerAddress, uint16_t registerValue, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理写多个线圈状态*/
static uint16_t HandleWriteMultipleCoilCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*处理写多个寄存器状态*/
static uint16_t HandleWriteMultipleRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);

uint16_t (*HandleMasterCommand[])(int, uint16_t, uint16_t, uint8_t *, uint8_t *) = {HandleReadCoilStatusCommand,
HandleReadInputStatusCommand,
HandleReadHoldingRegisterCommand,
HandleReadInputRegisterCommand,
HandleWriteSingleCoilCommand,
HandleWriteSingleRegisterCommand,
HandleWriteMultipleCoilCommand,
HandleWriteMultipleRegisterCommand};

/*解析接收到的信息，并返回合成的回复信息和信息的字节长度，通过回调函数*/
/*
receivedMessage	:接收数据缓冲区
respondBytes	:数据响应缓冲区
rxLength		:接收数据长度
StationAddress	:本站地址
*/
uint16_t ParsingMasterAccessCommand(int DevNo, uint8_t *receivedMessage, uint8_t *respondBytes, uint16_t rxLength, uint8_t StationAddress)
{
  uint16_t respondLength = 0;
  /*判断是否是本站，如不是不处理*/
  uint8_t slaveAddress = *receivedMessage;
  if (slaveAddress != StationAddress)
  {
    return 0;
  }
  /*判断功能码是否有误*/
  FunctionCode fc = (FunctionCode)(*(receivedMessage + 1));
  if (CheckFunctionCode(fc) != Modbus_OK)
  {
    return 0;
  }

  /*信息校验，如不正确则为错误信息不用处理*/
  uint16_t dataLength = 8;
  if ((fc == WriteMultipleCoil) || (fc == WriteMultipleRegister))
  {
    dataLength = (uint16_t)(*(receivedMessage + 6)) + 9;
    if (rxLength < dataLength) //尚未接收完整
    {
      return 65535;
    }
  }
  bool chechMessageNoError = CheckRTUMessageIntegrity(receivedMessage, dataLength);
  if (!chechMessageNoError)
  {
    return 0;
  }
  
  uint16_t startAddress = (uint16_t)(*(receivedMessage + 2));
  startAddress = (startAddress << 8) + (uint16_t)(*(receivedMessage + 3));
  uint16_t quantity = (uint16_t)(*(receivedMessage + 4));
  quantity = (quantity << 8) + (uint16_t)(*(receivedMessage + 5));
  
  uint8_t index = (fc > 0x08) ? (fc - 0x09) : (fc - 0x01);
  
  respondLength = HandleMasterCommand[index](DevNo, startAddress, quantity, receivedMessage, respondBytes);
  return respondLength;
}

/*处理读线圈状态命令*/
static uint16_t HandleReadCoilStatusCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool statusList[250];
  int i;
  memset(statusList,0,sizeof(bool)*250);
  if(quantity > 128){
  	perror("quantity > 128\n");
	return length = 0;
  } 
  GetCoilStatus(DevNo, startAddress, quantity, statusList);
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, statusList, NULL, respondBytes);
  
  return length;
}

/*处理读输入状态命令*/
static uint16_t HandleReadInputStatusCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool statusList[128];
  if(quantity > 128){
  	perror("quantity > 128\n");
	return length = 0;
  } 
  GetInputStatus(DevNo, startAddress, quantity, statusList);
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, statusList, NULL, respondBytes);
  
  return length;
}

/*处理读保持寄存器命令,功能码03*/
static uint16_t HandleReadHoldingRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  uint16_t registerList[128];
  memset(registerList,0,128*2);
  if(quantity > 128){
  	perror("quantity > 128\n");
	return length = 0;
  } 
  GetHoldingRegister(DevNo, startAddress, quantity, registerList);
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, NULL, registerList, respondBytes);
  return length;
}

/*处理读输入寄存器命令*/
static uint16_t HandleReadInputRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  
  uint16_t registerList[128];
  memset(registerList,0,128*2);
  if(quantity > 128){
  	perror("quantity > 128\n");
	return length = 0;
  } 
  GetInputRegister(DevNo, startAddress, quantity, registerList);
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, NULL, registerList, respondBytes);
  
  return length;
}

/*处理写单个线圈命令*/
static uint16_t HandleWriteSingleCoilCommand(int DevNo, uint16_t coilAddress, uint16_t coilValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool value;
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, NULL, NULL, respondBytes);
  GetCoilStatus(DevNo, coilAddress, 1, &value);
  
  value = CovertSingleCommandCoilToBoolStatus(coilValue, value);
  SetSingleCoil(DevNo, coilAddress, value);
  
  return length;
}

/*处理写单个寄存器命令*/
static uint16_t HandleWriteSingleRegisterCommand(int DevNo, uint16_t registerAddress, uint16_t registerValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, NULL, NULL, respondBytes);
  
  SetSingleRegister(DevNo, registerAddress, registerValue);
  
  return length;
}

/*处理写多个线圈状态*/
static uint16_t HandleWriteMultipleCoilCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool statusValue[250];
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, statusValue, NULL, respondBytes);
  
  SetMultipleCoil(DevNo, startAddress, quantity, statusValue);
  
  return length;
}

/*处理写多个寄存器状态*/
static uint16_t HandleWriteMultipleRegisterCommand(int DevNo, uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  uint16_t registerValue[125];
  
  length = SyntheticSlaveAccessRespond(DevNo, receivedMessage, NULL, registerValue, respondBytes);
  
  SetMultipleRegister(DevNo, startAddress, quantity, registerValue);
  return length;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
