/******************************************************************************/
/** 模块名称：Modbus通讯                                                     **/
/** 文件名称：mbcommon.h                                                     **/
/** 版    本：V1.0.0                                                         **/
/** 简    介：用于实现Modbus各种情况下的公用部分                             **/
/**--------------------------------------------------------------------------**/
/** 修改记录：                                                               **/
/**     版本      日期              作者              说明                   **/
/**     V1.0.0  2015-07-18          木南              创建文件               **/
/**                                                                          **/
/******************************************************************************/ 

#ifndef __mbcommon_h
#define __mbcommon_h

#include "stdbool.h"
#include "stdint.h"

/*定义更新读取回来的对象值的函数指针类型*/
/*更新读回来的线圈状态*/
typedef void (*UpdateCoilStatusType)(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*更新读回来的输入状态值*/
typedef void (*UpdateInputStatusType)(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*更新读回来的保持寄存器*/
typedef void (*UpdateHoldingRegisterType)(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*更新读回来的输入寄存器*/
typedef void (*UpdateInputResgisterType)(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*将接收到的写单个Coil值转化为布尔量，对应0x05功能码*/
bool CovertSingleCommandCoilToBoolStatus(uint16_t coilValue,bool value);

/*检验所写数据是否符合物理量要求范围并处理(单精度浮点数)*/
float CheckWriteFloatDataIsValid(float value,float range,float zero);
 
 /*检验所写数据是否符合物理量要求范围并处理(双精度浮点数)*/
double CheckWriteDoubleDataIsValid(double value,double range,double zero);

/*检验所写数据是否符合物理量要求范围并处理(16位整数)*/
uint16_t CheckWriteInt16DataIsValid(uint16_t value,uint16_t range,uint16_t zero);
 
 /*检验所写数据是否符合物理量要求范围并处理(32位整数)*/
uint32_t CheckWriteInt32DataIsValid(uint32_t value,uint32_t range,uint32_t zero);

/*获取想要读取的Coil量的值*/
void GetCoilStatus(int DevID, uint16_t startAddress,uint16_t quantity,bool *statusList);

/*获取想要读取的InputStatus量的值*/
void GetInputStatus(int DevID, uint16_t startAddress,uint16_t quantity,bool *statusValue);

/*获取想要读取的保持寄存器的值*/
void GetHoldingRegister(int DevID, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*获取想要读取的输入寄存器的值*/
void GetInputRegister(int DevID, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*设置单个线圈的值*/
void SetSingleCoil(int DevID, uint16_t coilAddress,bool coilValue);

/*设置单个寄存器的值*/
void SetSingleRegister(int DevID, uint16_t registerAddress,uint16_t registerValue);

/*设置多个线圈的值*/
void SetMultipleCoil(int DevID, uint16_t startAddress,uint16_t quantity,bool *statusValue);

/*设置多个寄存器的值*/
void SetMultipleRegister(int DevID, uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*更新读回来的线圈状态*/
void UpdateCoilStatus(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*更新读回来的输入状态值*/
void UpdateInputStatus(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*更新读回来的保持寄存器*/
void UpdateHoldingRegister(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*更新读回来的输入寄存器*/
void UpdateInputResgister(int DevID, uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

#endif //__mbcommon_h
/*********** (C) COPYRIGHT 1999-2019 Moonan Technology *********END OF FILE****/