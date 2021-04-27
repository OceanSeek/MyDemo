/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�modbusrtuslave.h                                               **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus RTU��վ������Լ�����                           **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ľ��              �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#ifndef __mbrtuslave_h
#define __mbrtuslave_h

#include "mbrtu.h"
#include "mbcommon.h"


/*�������յ�����Ϣ�������غϳɵĻظ���Ϣ����Ϣ���ֽڳ��ȣ�ͨ���ص�����*/
uint16_t ParsingMasterAccessCommand(int DevID, uint8_t *receivedMesasage,uint8_t *respondBytes,uint16_t rxLength,uint8_t StationAddress);

#endif
/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/