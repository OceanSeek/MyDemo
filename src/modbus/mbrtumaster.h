/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�modbusrtumaster.h                                              **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus RTU��վ������Լ�����                           **/
/**           1�����ͷ�������                                                **/
/**           2������������Ϣ                                                **/
/**           3�����ݷ�����Ϣ�޸�����                                        **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ľ��              �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#ifndef __mbrtumaster_h
#define __mbrtumaster_h

#include "mbrtu.h"
#include "mbcommon.h"

/* ���屻����RTU��վ�������� */
typedef struct AccessedRTUSlaveType{
  uint8_t stationAddress;       //վ��ַ
  uint8_t cmdOrder;             //��ǰ�����������б��е�λ��
  uint16_t commandNumber;       //�����б������������
  uint8_t (*pReadCommand)[8];   //�������б�
  uint8_t *pLastCommand;        //��һ�η��͵�����
  uint32_t flagPresetCoil;      //Ԥ����Ȧ���Ʊ�־λ
  uint32_t flagPresetReg;       //Ԥ�üĴ������Ʊ�־λ
}RTUAccessedSlaveType;

/* ���屾��RTU��վ�������� */
typedef struct LocalRTUMasterType{
  uint32_t flagWriteSlave[8];   //дһ��վ���Ʊ�־λ�����256��վ����վ��ַ��Ӧ��
  uint16_t slaveNumber;         //��վ�б��д�վ������
  uint16_t readOrder;           //��ǰ��վ�ڴ�վ�б��е�λ��
  RTUAccessedSlaveType *pSlave;         //��վ�б�
  UpdateCoilStatusType pUpdateCoilStatus;       //������Ȧ������
  UpdateInputStatusType pUpdateInputStatus;     //��������״̬������
  UpdateHoldingRegisterType pUpdateHoldingRegister;     //���±��ּĴ���������
  UpdateInputResgisterType pUpdateInputResgister;       //��������Ĵ���������
}RTULocalMasterType;

typedef struct{
  uint16_t _RegAddr;//
  uint16_t _SlaverAddr;//
  uint8_t _DataType;
  uint8_t _DataOffset;
}MODBUSDEVICEDATA_T,*PMODBUSDEVICEDATA_T;


/* ������������ */
typedef struct{
  bool *pCoilStatus;   //��Ȧ״̬
  bool *pInputStatus;	//����״̬
  uint16_t *pHoldingRegister;         //���ּĴ���
  uint16_t *pInputResgister;           //����Ĵ���
  uint16_t _CoilStatusNum;//��Ȧ����
  uint16_t _InputStatusNum;//��Ȧ����
  uint16_t _HoldingRegNum;//���ּĴ�������
  uint16_t _InputRegNum;//����Ĵ�������
  uint16_t _CoiStatusStart;
  uint16_t _InputStatusStart;
  uint16_t _HoldingRegStart;
  uint16_t _InputRegStart;
  uint16_t _tcpTranslate;//tcp事务标志
  bool *pFlag_AskCoilStatus;//�ٻ����ݱ�־
  bool *pFlag_AskInputStatus;
  bool *pFlag_AskHoldingRegister;
  bool *pFlag_AskInputResgister;

  bool *pFlag_WriteSingleHoldingRegister;
  uint16_t *pWriteSingleHoldingRegisterValue;

  MODBUSDEVICEDATA_T *pCoiStatus_T;
  MODBUSDEVICEDATA_T *pInputStatus_T;
  MODBUSDEVICEDATA_T *pHoldingRegister_T;
  MODBUSDEVICEDATA_T *pInputResgister_T;

  
  
  
}ModbusData_T,*PModbusData_T;

/*���ɷ��ʷ�����������*/
uint16_t CreateAccessSlaveCommand(int DevID, ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes);

/*�����յ��ķ�������Ӧ��Ϣ*/
void ParsingSlaveRespondMessage(int DevID, RTULocalMasterType *master,uint8_t *recievedMessage,uint8_t *command);

/*���յ�������Ϣ���ж��Ƿ��Ƿ��������б�������ķ�����Ϣ*/
// static int FindCommandForRecievedMessage(int DevID, uint8_t *recievedMessage,uint8_t (*commandList)[8],uint16_t commandNumber);

/* ʹ�ܻ���ʧ��д��վ������־λ���޸Ĵ�վ��дʹ�ܱ�־λ�� */
void ModifyWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress,bool en);

/* ��ô�վ��дʹ�ܱ�־λ��״̬ */
bool GetWriteRTUSlaveEnableFlag(RTULocalMasterType *master,uint8_t slaveAddress);

/*��ʼ��RTU��վ����*/
void InitializeRTUMasterObject(RTULocalMasterType *master,uint16_t slaveNumber,
                            RTUAccessedSlaveType *pSlave,
                            UpdateCoilStatusType pUpdateCoilStatus,
                            UpdateInputStatusType pUpdateInputStatus,
                            UpdateHoldingRegisterType pUpdateHoldingRegister,
                            UpdateInputResgisterType pUpdateInputResgister
                            );

/* �жϵ�ǰ�Ƿ���д����ʹ�� */
bool CheckWriteRTUSlaveNone(RTULocalMasterType *master);


#endif
/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
