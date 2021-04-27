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

#include "sys.h"

/*处理读从站状态量返回信息，读线圈状态位0x01功能码*/
static void HandleReadCoilStatusRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*处理读从站状态量返回信息，读输入状态位0x02功能码*/
static void HandleReadInputStatusRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*处理读从站寄存器值的返回信息，读保持寄存器0x03功能码）*/
static void HandleReadHoldingRegisterRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*处理读从站寄存器值的返回信息，读输入寄存器0x04功能码*/
static void HandleReadInputRegisterRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*判断接收到的信息是否是发送命令的返回信息*/
static bool CheckMessageAgreeWithCommand(int DevNo, uint8_t *recievedMessage,uint8_t *command);

static int FindCommandForRecievedMessage(int DevNo, uint8_t *recievedMessage,uint8_t (*commandList)[8],uint16_t commandNumber);


void (*HandleSlaveRespond[])(int, RTULocalMasterType *,uint8_t *,uint16_t,uint16_t)={HandleReadCoilStatusRespond,
HandleReadInputStatusRespond,
HandleReadHoldingRegisterRespond,
HandleReadInputRegisterRespond};

/*函数名：CreateAccessSlaveCommand，生成访问服务器的命令*/
/*参数：ObjAccessInfo objInfo,要生成访问命令的对象信息*/
/*      void *dataList,写的数据列表，寄存器为uint16_t类型，状态量为bool类型*/
/*      uint8_t *commandBytes,生成的命令列表*/
/*返回值：uint16_t，生成的命令的长度*/
uint16_t CreateAccessSlaveCommand(int DevNo, ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes)
{
  uint16_t commandLength=0;
  /*生成读服务器对象的命令，功能码0x01、0x02、0x03、0x04,命令长度8个字节*/
  if((objInfo.functionCode>=ReadCoilStatus)&&(objInfo.functionCode <= ReadInputRegister))
  {
    commandLength=SyntheticReadWriteSlaveCommand(DevNo, objInfo,NULL,NULL,commandBytes);
  }
  
  /*生成预置服务器对象的命令，功能码0x05,0x0F,命令长度随发送数据而变*/
  if((objInfo.functionCode==WriteSingleCoil)||(objInfo.functionCode==WriteMultipleCoil))
  {
    bool *statusList=(bool*)dataList;
    commandLength=SyntheticReadWriteSlaveCommand(DevNo, objInfo,statusList,NULL,commandBytes);
  }
  
  /*生成预置服务器对象的命令，功能码0x06,0x10,命令长度随发送数据而变*/
  if((objInfo.functionCode==WriteSingleRegister)||(objInfo.functionCode==WriteMultipleRegister))
  {
    uint16_t *registerList=(uint16_t*)dataList;
    commandLength=SyntheticReadWriteSlaveCommand(DevNo, objInfo,NULL,registerList,commandBytes);
  }
  
  return commandLength;
}

/*解析收到的服务器相应信息*/
/*uint8_t *recievedMessage,接收到的消息列表*/
/*uint8_t *command,发送的读操作命令，若为NULL则在命令列表中查找*/
void ParsingSlaveRespondMessage(int DevNo, RTULocalMasterType *master,uint8_t *recievedMessage,uint8_t *command)
{
  int i=0;
  int j=0;
  uint16_t startAddress;
  uint16_t quantity;
  uint8_t *cmd=NULL;
  
  /*如果不是读操作的反回信息不需要处理*/
  if(recievedMessage[1]>0x04)
  {
    return;
  }
  
  /*判断功能码是否有误*/
  FunctionCode fuctionCode=(FunctionCode)recievedMessage[1];
  if (CheckFunctionCode(fuctionCode) != Modbus_OK)
  {
    return;
  }
  
  /*校验接收到的信息是否有错*/
  uint16_t byteCount=recievedMessage[2];
  bool chechMessageNoError=CheckRTUMessageIntegrity(recievedMessage,byteCount+5);
  if(!chechMessageNoError)
  {
    return;
  }
  if((command==NULL)||(!CheckMessageAgreeWithCommand(DevNo, recievedMessage,command)))
  {
    while(i<master->slaveNumber)
    {
      if(master->pSlave[i].stationAddress==recievedMessage[0])
      {
        break;
      }
      i++;
    }
    if(i>=master->slaveNumber)
    {
      return;
    }
    if((master->pSlave[i].pLastCommand==NULL)||(!CheckMessageAgreeWithCommand(DevNo, recievedMessage,master->pSlave[i].pLastCommand)))
    {
      j=FindCommandForRecievedMessage(DevNo, recievedMessage,master->pSlave[i].pReadCommand,master->pSlave[i].commandNumber);
      if(j<0)
      {
        return;
      }
      cmd=master->pSlave[i].pReadCommand[j];
    }
    else
    {
      cmd=master->pSlave[i].pLastCommand;
    }
  }
  else
  {
    cmd=command;
  }
  
  startAddress=(uint16_t)cmd[2];
  startAddress=(startAddress<<8)+(uint16_t)cmd[3];
  quantity=(uint16_t)cmd[4];
  quantity=(quantity<<8)+(uint16_t)cmd[5];
  
  if((fuctionCode>=ReadCoilStatus)&&(fuctionCode<=ReadInputRegister))
  {
    HandleSlaveRespond[fuctionCode-1](DevNo, master,recievedMessage,startAddress,quantity);
  }
}

/*接收到返回信息后，判断是否是发送命令列表中命令的返回信息，*/
/*若是则从已发送命令列表中删除命令，若不是则丢弃该条返回信息*/
static int FindCommandForRecievedMessage(int DevNo, uint8_t *recievedMessage,uint8_t (*commandList)[8],uint16_t commandNumber)
{
  int cmdIndex=-1;
  int i;
  for(i=0;i<commandNumber;i++)
  {
    if(CheckMessageAgreeWithCommand(DevNo, recievedMessage,commandList[i])==true)
    {
      cmdIndex=i;
      break;
    }
  }
  return cmdIndex;
}

/*判断接收到的信息是否是发送命令的返回信息*/
static bool CheckMessageAgreeWithCommand(int DevNo, uint8_t *recievedMessage,uint8_t *command)
{
  bool aw=false;
  
  if((recievedMessage[0]==command[0])&&(recievedMessage[1]==command[1]))
  {
    uint16_t quantity=(uint16_t)command[4];
    quantity=(quantity<<8)+(uint16_t)command[5];
    uint8_t bytescount=0;
    if((recievedMessage[1]==ReadCoilStatus)||(recievedMessage[1]==ReadInputStatus))
    {
      bytescount=(uint8_t)((quantity-1)/8+1);
    }
    
    if((recievedMessage[1]==ReadHoldingRegister)||(recievedMessage[1]==ReadInputRegister))
    {
      bytescount=quantity*2;
    }
    
    if(recievedMessage[2]==bytescount)
    {
      aw=true;
    }
  }
  
  return aw;
}
/*处理读从站状态量返回信息，读线圈状态位0x012功能码*/
static void HandleReadCoilStatusRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  bool coilStatus[256];
  
  TransformClientReceivedData(DevNo, receivedMessage,quantity,coilStatus,NULL);
  
  uint8_t slaveAddress=receivedMessage[0];
  
  master->pUpdateCoilStatus(DevNo, slaveAddress,startAddress,quantity,coilStatus);
  
}

/*处理读从站状态量返回信息，读输入状态位0x02功能码*/
static void HandleReadInputStatusRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  bool inputStatus[256];
  
  TransformClientReceivedData(DevNo, receivedMessage,quantity,inputStatus,NULL);
  
  uint8_t slaveAddress=receivedMessage[0];
  
  master->pUpdateInputStatus(DevNo, slaveAddress,startAddress,quantity,inputStatus);
}

/*处理读从站寄存器值的返回信息，读保持寄存器0x03功能码）*/
static void HandleReadHoldingRegisterRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
	
  uint16_t holdingRegister[256];
  TransformClientReceivedData(DevNo, receivedMessage,quantity,NULL,holdingRegister);
  uint8_t slaveAddress=receivedMessage[0];
  master->pUpdateHoldingRegister(DevNo, slaveAddress,startAddress,quantity,holdingRegister);
}

/*处理读从站寄存器值的返回信息，读输入寄存器0x04功能码*/
static void HandleReadInputRegisterRespond(int DevNo, RTULocalMasterType *master,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  uint16_t inputRegister[256];
  TransformClientReceivedData(DevNo, receivedMessage,quantity,NULL,inputRegister);
  uint8_t slaveAddress=receivedMessage[0];
  master->pUpdateInputResgister(DevNo, slaveAddress,startAddress,quantity,inputRegister);
}

/*初始化RTU主站对象*/
void InitializeRTUMasterObject(RTULocalMasterType *master,uint16_t slaveNumber,
                               RTUAccessedSlaveType *pSlave,
                               UpdateCoilStatusType pUpdateCoilStatus,
                               UpdateInputStatusType pUpdateInputStatus,
                               UpdateHoldingRegisterType pUpdateHoldingRegister,
                               UpdateInputResgisterType pUpdateInputResgister
                                 )
{
  int i;
  master->slaveNumber=slaveNumber>255?255:slaveNumber;
  
  master->readOrder=0;
  
  master->pSlave=pSlave;
  
  for(i=0;i<8;i++)
  {
    master->flagWriteSlave[i]=0x00000000;
  }
  
  master->pUpdateCoilStatus=(pUpdateCoilStatus!=NULL)?pUpdateCoilStatus:UpdateCoilStatus;
  
  
  master->pUpdateInputStatus=(pUpdateInputStatus!=NULL)?pUpdateInputStatus:UpdateInputStatus;
  
  master->pUpdateHoldingRegister=(pUpdateHoldingRegister!=NULL)?pUpdateHoldingRegister:UpdateHoldingRegister;
  
  master->pUpdateInputResgister=(pUpdateInputResgister!=NULL)?pUpdateInputResgister:UpdateInputResgister;
}

/* 使能或者失能写从站操作标志位（修改从站的写使能标志位） */
void ModifyWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress,bool en)
{
  uint8_t row=0;
  uint8_t column=0;
  
  row=slaveAddress/32;
  column=slaveAddress%32;
  
  if(en)
  {
    master->flagWriteSlave[row]|=(0x00000001<<column);
  }
  else
  {
    master->flagWriteSlave[row]&=(~(0x00000001<<column));
  }
}

/* 获得从站的写使能标志位的状态 */
bool GetWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress)
{
  bool status=false;
  uint8_t row=0;
  uint8_t column=0;
  
  row=slaveAddress/32;
  column=slaveAddress%32;
  
  if((master->flagWriteSlave[row]&(0x00000001<<column))==(0x00000001<<column))
  {
    status=true;
  }
  
  return status;
}

/* 判断当前是否有写操作使能 */
bool CheckWriteRTUSlaveNone(RTULocalMasterType *master)
{
  bool status=true;
  int i;
  
  for(i=0;i<8;i++)
  {
    if(master->flagWriteSlave[i]>0x00000000)
    {
      status=false;
    }
  }
  
  return status;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
