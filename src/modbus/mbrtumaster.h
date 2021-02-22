/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：modbusrtumaster.h                                              **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于声明Modbus RTU主站相关属性及方法                           **/
/**           1、发送访问请求                                                **/
/**           2、解析返回信息                                                **/
/**           3、根据返回信息修改数据                                        **/
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2016-04-17          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/

#ifndef __mbrtumaster_h
#define __mbrtumaster_h

#include "mbrtu.h"
#include "mbcommon.h"

/* 定义被访问RTU从站对象类型 */
typedef struct AccessedRTUSlaveType{
  uint8_t stationAddress;       //站地址
  uint8_t cmdOrder;             //当前命令在命令列表中的位置
  uint16_t commandNumber;       //命令列表中命令的总数
  uint8_t (*pReadCommand)[8];   //读命令列表
  uint8_t *pLastCommand;        //上一次发送的命令
  uint32_t flagPresetCoil;      //预置线圈控制标志位
  uint32_t flagPresetReg;       //预置寄存器控制标志位
}RTUAccessedSlaveType;

/* 定义本地RTU主站对象类型 */
typedef struct LocalRTUMasterType{
  uint32_t flagWriteSlave[8];   //写一个站控制标志位，最多256个站，与站地址对应。
  uint16_t slaveNumber;         //从站列表中从站的数量
  uint16_t readOrder;           //当前从站在从站列表中的位置
  RTUAccessedSlaveType *pSlave;         //从站列表
  UpdateCoilStatusType pUpdateCoilStatus;       //更新线圈量函数
  UpdateInputStatusType pUpdateInputStatus;     //更新输入状态量函数
  UpdateHoldingRegisterType pUpdateHoldingRegister;     //更新保持寄存器量函数
  UpdateInputResgisterType pUpdateInputResgister;       //更新输入寄存器量函数
}RTULocalMasterType;

typedef struct{
  uint16_t _RegAddr;//寄存器地址
  uint16_t _SlaverAddr;//从机地址
}MODBUSDEVICEDATA_T,*PMODBUSDEVICEDATA_T;


/* 定义数据类型 */
typedef struct{
  bool *pCoilStatus;   //线圈状态
  bool *pInputStatus;	//输入状态
  uint16_t *pHoldingRegister;         //保持寄存器
  uint16_t *pInputResgister;           //输入寄存器
  uint8_t _CoilStatusNum;//线圈数量
  uint8_t _InputStatusNum;//线圈数量
  uint8_t _HoldingRegNum;//保持寄存器数量
  uint8_t _InputRegNum;//输入寄存器数量
  uint16_t _CoiStatusStart;
  uint16_t _InputStatusStart;
  uint16_t _HoldingRegStart;
  uint16_t _InputRegStart;
  bool *pFlag_AskCoilStatus;//召唤数据标志
  bool *pFlag_AskInputStatus;
  bool *pFlag_AskHoldingRegister;
  bool *pFlag_AskInputResgister;
  MODBUSDEVICEDATA_T *pCoiStatus_T;
  MODBUSDEVICEDATA_T *pInputStatus_T;
  MODBUSDEVICEDATA_T *pHoldingRegister_T;
  MODBUSDEVICEDATA_T *pInputResgister_T;
  
  
  
}ModbusData_T,*PModbusData_T;

/*生成访问服务器的命令*/
uint16_t CreateAccessSlaveCommand(int DevID, ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes);

/*解析收到的服务器相应信息*/
void ParsingSlaveRespondMessage(int DevID, RTULocalMasterType *master,uint8_t *recievedMessage,uint8_t *command);

/*接收到返回信息后，判断是否是发送命令列表中命令的返回信息*/
int FindCommandForRecievedMessage(int DevID, uint8_t *recievedMessage,uint8_t (*commandList)[8],uint16_t commandNumber);

/* 使能或者失能写从站操作标志位（修改从站的写使能标志位） */
void ModifyWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress,bool en);

/* 获得从站的写使能标志位的状态 */
bool GetWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress);

/*初始化RTU主站对象*/
void InitializeRTUMasterObject(RTULocalMasterType *master,uint16_t slaveNumber,
                            RTUAccessedSlaveType *pSlave,
                            UpdateCoilStatusType pUpdateCoilStatus,
                            UpdateInputStatusType pUpdateInputStatus,
                            UpdateHoldingRegisterType pUpdateHoldingRegister,
                            UpdateInputResgisterType pUpdateInputResgister
                            );

/* 判断当前是否有写操作使能 */
bool CheckWriteRTUSlaveNone(RTULocalMasterType *master);


#endif
/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
