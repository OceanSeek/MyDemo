/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：mbtcp.c                                                        **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于实现Modbus TCP总线协议站ADU的封装                          **/
/**           1、实现作为客户端时访问服务器数据命令的生成                    **/
/**           2、实现作为服务器端时，对客户端命令的响应信息的生成            **/
/**                                                                          **/
/* MBAP报文头的包括的内容：                                                   */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |     域      |  长度   | 客户机 | 服务器 |              描述            | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |事务元标识符 | 2个字节 |  启动  |  复制  |请求/响应事务处理的识别码     | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |协议标识符   | 2个字节 |  启动  |  复制  |0=MODBUS 协议                 | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |    长度     | 2个字节 |  启动  |  启动  |以下字节的数量                | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |单元标识符   | 1个字节 |  启动  |  复制  |连接的远程从站的识别码        | */
/* +-------------+---------+--------+---------------------------------------+ */
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2016-04-17          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/ 
#include "sys.h"

/*生成MBAP头数据,为了方便与RTU相同处理，将单元标识符（远程从站地址）放在生成命令时处理*/
/*此处MBAP头只有7个字节，最后单元标识符为默认值0x01*/
static uint16_t CreateMbapHeadPart(int DevNo, uint8_t * mbapHead)
{
  uint16_t index=0;
  //产生事务标志
  if(gpDevice[DevNo].ModbusData._tcpTranslate==65535)
  {
    gpDevice[DevNo].ModbusData._tcpTranslate=1;
  }
  else
  {
    gpDevice[DevNo].ModbusData._tcpTranslate++;
  }
  mbapHead[index++]=gpDevice[DevNo].ModbusData._tcpTranslate>>8;//事务标识符
  mbapHead[index++]=gpDevice[DevNo].ModbusData._tcpTranslate;//事务标识符
  mbapHead[index++]=0x00;//协议标识符，modbus标识符为0
  mbapHead[index++]=0x00;//协议标识符，modbus标识符为0
  return index;
}

/*生成读写服务器对象的命令*/
uint16_t SyntheticReadWriteTCPServerCommand(int DevNo, ObjAccessInfo objInfo,bool *statusList,uint16_t *registerList,uint8_t *commandBytes)
{
  uint8_t command[256];
  CreateMbapHeadPart(DevNo, commandBytes);
  
  uint16_t index=4;
  uint16_t bytesCount=GenerateReadWriteCommand(DevNo, objInfo,statusList,registerList,command);
  commandBytes[index++]=bytesCount>>8;
  commandBytes[index++]=bytesCount;	
  for(int i=0;i<bytesCount;i++)
  {
    commandBytes[index++]=command[i];
  }
  return index;
}

/*合成对服务器访问的响应,返回值为命令长度*/
uint16_t SyntheticServerAccessRespond(int DevNo, uint8_t *receivedMessage,bool *statusList,uint16_t *registerList,uint8_t *respondBytes)
{
  uint16_t index=0;
  respondBytes[index++]=*receivedMessage;
  respondBytes[index++]=*(receivedMessage+1);
  respondBytes[index++]=*(receivedMessage+2);
  respondBytes[index++]=*(receivedMessage+3);
  uint8_t respond[260];
  uint16_t bytesCount=GenerateMasterAccessRespond(DevNo, receivedMessage+6,statusList,registerList,respond);
  respondBytes[index++]=(bytesCount>>8);
  respondBytes[index++]=bytesCount;
  
  for(int i=0;i<bytesCount;i++)
  {
    respondBytes[index++]=respond[i];
  }
  return index;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/