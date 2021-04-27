/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：mbtcpserver.c                                                  **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于实现Modbus TCP服务器端的相关属性及方法                     **/
/**           1、初始化作为Server的相关参数，如存储域等                      **/
/**           2、解析接收到的消息，并对数据对象进行相应操作，返回响应消息    **/
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2016-04-17          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/ 
// #include "sys.h"
#include "mbtcpserver.h"

/*处理读线圈状态命令*/
static uint16_t HandleReadCoilStatusCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理读输入状态命令*/
static uint16_t HandleReadInputStatusCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理读保持寄存器命令*/
static uint16_t HandleReadHoldingRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理读输入寄存器命令*/
static uint16_t HandleReadInputRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理写单个线圈命令*/
static uint16_t HandleWriteSingleCoilCommand(int DevNo, uint16_t coilAddress,uint16_t coilValue,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理写单个寄存器命令*/
static uint16_t HandleWriteSingleRegisterCommand(int DevNo, uint16_t registerAddress,uint16_t registerValue,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理写多个线圈状态*/
static uint16_t HandleWriteMultipleCoilCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*处理写多个寄存器状态*/
static uint16_t HandleWriteMultipleRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);

uint16_t (*HandleClientCommand[])(int,uint16_t,uint16_t,uint8_t *,uint8_t *)={HandleReadCoilStatusCommand,
HandleReadInputStatusCommand,
HandleReadHoldingRegisterCommand,
HandleReadInputRegisterCommand,
HandleWriteSingleCoilCommand,
HandleWriteSingleRegisterCommand,
HandleWriteMultipleCoilCommand,
HandleWriteMultipleRegisterCommand};

/*解析接收到的信息，返回响应命令的长度*/
uint16_t ParsingClientAccessCommand(int DevNo, uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  FunctionCode fc=(FunctionCode)(*(receivedMessage+7));
  if(CheckFunctionCode(fc)!=Modbus_OK)
  {
    return 0;
  }
  
  uint16_t startAddress=(uint16_t)(*(receivedMessage+8));
  startAddress=(startAddress<<8)+(uint16_t)(*(receivedMessage+9));
  uint16_t quantity=(uint16_t)(*(receivedMessage+10));
  quantity=(quantity<<8)+(uint16_t)(*(receivedMessage+11));
  
  uint8_t index=(fc>0x08)?(fc-0x09):(fc-0x01);
  
  length=HandleClientCommand[index](DevNo,startAddress,quantity,receivedMessage,respondBytes);
  
  return length;
}

/*处理读线圈状态命令*/
static uint16_t HandleReadCoilStatusCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool statusList[250];
  
  GetCoilStatus(DevNo, startAddress,quantity,statusList);
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,statusList,NULL,respondBytes);
  
  return length;
}

/*处理读输入状态命令*/
static uint16_t HandleReadInputStatusCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  bool statusList[250];
  
  GetInputStatus(DevNo, startAddress,quantity,statusList);
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,statusList,NULL,respondBytes);
  
  return length;
}

/*处理读保持寄存器命令*/
static uint16_t HandleReadHoldingRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  uint16_t registerList[125];
  
  GetHoldingRegister(DevNo, startAddress,quantity,registerList);
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,NULL,registerList,respondBytes);
  
  return length;
}

/*处理读输入寄存器命令*/
static uint16_t HandleReadInputRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  uint16_t registerList[125];
  
  GetInputRegister(DevNo, startAddress,quantity,registerList);
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,NULL,registerList,respondBytes);
  
  return length;
}

/*处理写单个线圈命令*/
static uint16_t HandleWriteSingleCoilCommand(int DevNo, uint16_t coilAddress,uint16_t coilValue,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool value;
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,NULL,NULL,respondBytes);
  
  GetCoilStatus(DevNo, coilAddress,1,&value);
  
  value=CovertSingleCommandCoilToBoolStatus(coilValue,value);
  SetSingleCoil(DevNo, coilAddress,value);
  
  return length;
}

/*处理写单个寄存器命令*/
static uint16_t HandleWriteSingleRegisterCommand(int DevNo, uint16_t registerAddress,uint16_t registerValue,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,NULL,NULL,respondBytes);
  
  SetSingleRegister(DevNo, registerAddress,registerValue);
  
  return length;
}

/*处理写多个线圈状态*/
static uint16_t HandleWriteMultipleCoilCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool statusValue[250];
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,statusValue,NULL,respondBytes);
  
  SetMultipleCoil(DevNo, startAddress,quantity,statusValue);
  
  return length;
}

/*处理写多个寄存器状态*/
static uint16_t HandleWriteMultipleRegisterCommand(int DevNo, uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  uint16_t registerValue[125];
  
  length=SyntheticServerAccessRespond(DevNo, receivedMessage,NULL,registerValue,respondBytes);
  
  SetMultipleRegister(DevNo, startAddress,quantity,registerValue);
  return length;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/