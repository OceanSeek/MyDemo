#ifndef __siec101_2002_H__
#define __siec101_2002_H__

 

//ͷ�ļ�������      
//#include "CSlave.h"
//#include "cjptrlist.h"

BOOL Init_siec101_2002(int DevID);


#define USER_FLAG      32     //�û���־λ��
#define USER_SFLAG   32     //�û���־λ�� 
#define USER_SDWORDFLAG		8		//�û���DWORD���ݴ洢��
#define SYSTEM_SFLAG		0     //ϵͳ������־λ��


#define SF_HaveBO         (SYSTEM_SFLAG+0)    //��ң��
#define SF_HaveBOReturn   (SYSTEM_SFLAG+1)    //��ң�ط�У
#define SF_HaveAO         (SYSTEM_SFLAG+2)    //��ң��
#define SF_HaveAOReturn   (SYSTEM_SFLAG+3)    //��ң����У
#define SF_HaveTQ         (SYSTEM_SFLAG+4)    //��ͬ��
#define SF_HaveTQReturn   (SYSTEM_SFLAG+5)    //��ͬ�ڷ�У
#define SF_Time           (SYSTEM_SFLAG+6)    //���ʱ
#define SF_Proc           (SYSTEM_SFLAG+7)    //�Ѵ����־
#define SF_HaveDO         (SYSTEM_SFLAG+8)    //�����������
#define SF_HaveDOReturn   (SYSTEM_SFLAG+9)    //�������������У
#define SF_HaveCOE        (SYSTEM_SFLAG+10)    //�в���
//lgh--
#define SF_HaveDZ         (SYSTEM_SFLAG+14)    //�ж�ֵ
#define SF_HaveDZReturn   (SYSTEM_SFLAG+15)    //�ж�ֵ��У


typedef struct {

//�ڴ�����
	DWORD  dwMemBlockSize;   //�������ݴ���������С,��pGetMemBlock()ȡ�øû�������ַ

//ͨѶ����
//	DWORD  dwMaxSleepTime; //�ͨѶ�ж�ʱ��
	DWORD  dwTxdRetryCount;  //�ط�����
	DWORD  dwMaxErrCount;    //��������dwMaxErrCount�κ�֪ͨ���ݿ�
    DWORD  dwTxdBufSize;     //���ͻ�������С
	DWORD  dwRxdBufSize;     //���ջ�������С
	
//���ݿ��������
	DWORD  dwMinLogicDevNum;	 //��С�߼����豸��
	DWORD  dwMinRealDevNum;	 //��Сʵ�ʿ��豸��
	DWORD  dwGBAddress;      	 //�㲥���ĵ�ַ
	DWORD dwMaxAINum;
	DWORD dwMaxBINum;
	DWORD dwMaxCINum;
	DWORD dwMaxBONum;
	DWORD dwMaxAONum;
	DWORD dwMaxTQNum;
}TPMI;


#define FCV_BIT	0x10
#define FCB_BIT	0x20

#define TAILLEN	2


#define FILE_SELECT_OK		0x01
#define FILE_SECTION_OK		0x02

#define SENDKWHCALL		0
#define SENDKWHFREEZE	1

#define BCD_CMD_CODE	7
#define SPI_CMD_CODE	5


#define GRP_CINUM   32		//ÿ����෢�͵�ȸ���
#define GRPNUMCI	16		//������� 16*32=512

#define SF_RESET_PROCESS	(USER_SFLAG+1)	//����λRTU��־

#define SF_RtuInitOk		(USER_SFLAG+32)   //RTU��ʼ������
#define SF_SendInitFlag		(USER_SFLAG+33)	  //���ͳ�ʼ��������־

#define SF_CallData			(USER_SFLAG+34)		//Ŀǰ�����ٻ�����״̬
#define SF_CallCmdFlag		(USER_SFLAG+35)		//��Ӧ���ٻ�����
#define SF_ClockSynFlag		(USER_SFLAG+36)		//��Ӧ��������		
#define SF_SetParaFlag		(USER_SFLAG+37)		//��������
#define SF_CallDelayFlag	(USER_SFLAG+38)
#define SF_ClockDifferFlag	(USER_SFLAG+39)

#define SF_FILE_FLAG		(USER_SFLAG+40)		//�ļ����Է��ͱ�־���������Ӻ�Ҫ�����ļ�����֡���Ա�����ȷ������װ��������װ�ļ�
#define SF_ReadCIFlag		(USER_SFLAG+41)		//read ci flag
#define SF_MirrorFlag		(USER_SFLAG+42)		//��վ�ش���
#define SF_HaveScanAiData	(USER_SFLAG+43)		//��ɨ���ң������

#define SF_YkRtnInfoFlag	(USER_SFLAG+48)		//ң�ط��ر�־
#define SF_YkExitFlag		(USER_SFLAG+49)		//ң�ؽ�����־

#define SF_YtRtnInfoFlag	(USER_SFLAG+50)		//ң�ط��ر�־
#define SF_YtExitFlag		(USER_SFLAG+51)		//ң�ؽ�����־

#define SF_CALLDIR_FLAG		(USER_SFLAG+52)		//Ŀ¼�ٻ����
#define SF_FILE_SELECT		(USER_SFLAG+53)		//file select
#define SF_FILE_REQUEST		(USER_SFLAG+54)		//file request
#define SF_FILE_SENDING		(USER_SFLAG+55)		//file sending
#define SF_LAST_SEGMENT		(USER_SFLAG+56)		//last segment
#define SF_LAST_SECTION		(USER_SFLAG+57)		//last section

#define SF_CALLCIFLAG0		(USER_SFLAG+64)//���ٻ������ʼ���16*32=512

#define SF_RXDCONTROL		(USER_SDWORDFLAG + 0) //���ܵ��Ŀ�����
#define SF_NEXT_FCBFCV		(USER_SDWORDFLAG + 1)

#define SF_FILE_CRC			(USER_SDWORDFLAG + 2) //���յ����ļ���CRC, �´��ļ�ʱ�õ�
#define SF_RTU_STATUS		(USER_SDWORDFLAG + 3)//

#define SF_FILE_NAME		(USER_SDWORDFLAG + 4)
#define SF_SECTION_NAME		(USER_SDWORDFLAG + 5)
#define SF_SEGMENT_NAME		(USER_SDWORDFLAG + 6)
#define SF_FILE_OFFSET		(USER_SDWORDFLAG + 7)
#define SF_FILEOBJ_ADDR		(USER_SDWORDFLAG + 8)



//�궨�����ļ�������
#define MAX_TXDFM_SIZE  256  //�����֡��
#define MAX_RXDFM_SIZE  256  //������֡��
#define MIN_RXDFM_SIZE  5    //��С����֡��

//֡��������
#define BIT_DIR    0x80    //���䷽��λ
#define BIT_PRM    0x40    //��������λ
#define BIT_FCB    0x20    //֡����λ
#define BIT_FCV    0x10    //֡������Чλ
#define BIT_CODE   0x0F    //��������ռλ

#define BIT_FCBFCV	0x30

//�����붨��
#define  RESREMLIN 0x00 //��λԶ����·
#define  RESRTU    0x01 //��λԶ���ն�
#define  SENDAT    0x03 //��������
#define  REQANS    0x08 //�ٻ�Ҫ�����λ
#define  REQLINSTA 0x09 //����Զ����·״̬
#define  CALFIRDAT 0x0A //�ٻ�һ���û�����

#define FILE_READ_SELECT				131
#define FILE_WRITE_SELECT				132
#define FILE_READ_SELECT_RESPOND		133
#define FILE_WRITE_SELECT_RESPOND		134
#define FILE_READ_FILE					135
#define FILE_DATA_SEND					136
#define FILE_FILE_ACK					137
#define FILE_DIR_SEND					138

#define FILE_SEND_REASON				64
#pragma pack(push)
#pragma pack(1)


typedef struct 
{
	WORD wFileName;
	DWORD dwLen;
	BYTE bSOF;//�ļ�״̬
	WORD wMSecond;
	BYTE bMinute;
	BYTE bHour;
	BYTE bDay;
	BYTE bMonth;
	WORD wYear;
	char lzName[12];
}TFDRTA;


//0:spi 1:dpi 2:step 3:TBitString32 4:ai 5:kwh 6:tdo 7:TRegulate



typedef struct 
{
    BYTE bStart;	//�����ַ�
    BYTE bControl;	//������
	BYTE bAddress;//��·��ַ����վվַ��
    BYTE bChkSum;	//֡У���
    BYTE bStop;		//�����ַ�
}TRxdFixFm, TTxdFixFm; //I101S��Լ�Ĺ̶�֡���ṹ


typedef struct 
{
    BYTE bStart1;		//�����ַ�
    BYTE bLength1;		//����
    BYTE bLength2;		//�ظ�����
    BYTE bStart2;		//�����ַ�
    BYTE bControl;		//������
	BYTE bAddress1;		//��·��ַ����վվַ��  �ɱ� 1 or 2
    BYTE bType;			//���ͱ�ʶ
    BYTE bDefinitive;	//�ṹ�޶���
	BYTE bReason;		//����ԭ��		�ɱ� 1 or 2
	BYTE bAddress2;		//������ַ����վվַ���ɱ� 1 or 2
    BYTE bData;			//���ݿ�ʼ
}TRxdVarFm,TTxdVarFm;	//I101S��Լ�Ŀɱ�֡���ṹ

typedef union 
{
	TRxdFixFm fixFm;
	TRxdVarFm varFm;
}TSIEC101_2002RxdFm;		//101��Լ�Ľ���֡�ṹ��ͷ����

typedef union 
{
	TTxdFixFm fixFm;
	TTxdVarFm varFm;
}TSIEC101_2002TxdFm;			//��Լ�ķ���֡�ṹ��ͷ����

typedef struct
{
	DWORD dwStatus;
	DWORD dwLen;
	DWORD dwTime;
	WORD wCrc16;
	BYTE bNameLen;
	BYTE bName;
}TFileStatus;

typedef struct
{
	WORD wObjAddr;// = 0xf000
	BYTE bSFQ;
	DWORD dwFileID;
	DWORD dwOffset;
	BYTE bPacketLen;
	BYTE bData;
}TSendDataHead;

#pragma pack()


//TxdIdle��ϢʱӦ���͵�֡���TITF(m_Txd Idle Task Flag)


//class CSIEC101_2002 : public C_Slave  
//{
//        
//public:
//	CSIEC101_2002();
//    BOOL Init(void);//��ʼ��
//	
///*	struct XDIR xdir;
//	struct dirent direntry;
//	struct stat filestat;
//	CJPtrList m_dirList;
//*/
//	BOOL ScanDirFile();
//	void WriteInfoToFile(FILE* pFileID, TFDRTA* pTFDRTA);
//	void SizeToStr(DWORD dwSize, char* lzBuf);
//	BOOL CancelSelectFile(WORD wFileName);
//	BOOL TxdFileSending();
//	BOOL TxdLastSegment();
//
//
//	WORD m_wMinFrmLen;
//
//	BYTE* m_pRxdBuf;
//	BYTE* m_pTxdBuf;
//	
//	TAbsTime m_absTime1;
//	WORD m_wDelayTime;
//	WORD m_wFromTime;
//
//	BYTE m_bSendMirrorFlag;
//	BYTE m_bMirrorBuf[300];
//	void MakeMirror(BYTE bReason);
//	WORD m_wSelfInit;
//
//	void ScanAiData();
//
//protected:
//	BYTE m_bTmpBuf[20];
//	TIEC101Cfg* m_pCfg;
//	void ReadCfgFile();
//	void DefaultCfgFile();
//	void CheckCfgFile();
//
//    TFileStatus fileStatus;
//	TFileStatus* pFileStatus;
//
//	BYTE m_bRxdControl;
//
//	TSIEC101_2002RxdFm *pRxdFm; //����֡ͷָ��
//    TSIEC101_2002TxdFm *pTxdFm;//����֡ͷָ��
//
//	BOOL CheckAndRetry(BYTE bControl);
//
//	void SetALLDataFlag();
//	
//    void OnTimeOut(DWORD dwTimerID);
//    void OnBOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwBONo,DWORD dwResult);//ң�ط�У
//	void OnAOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwAONo,short AOValue,DWORD dwResult);//ң����У
//
//
//    DWORD SearchOneFrame(BYTE* pBuf, short nLen);
//
//	BOOL ChkVarControl(BYTE bCode);
//
//	
//	void SetKwhSendFlag(BOOL bAll);
//
//	BOOL RxdMonitor();
//	
//	BOOL RxdFixFrame();
//	BOOL RxdVarFrame();
//	BOOL RxdResetLink();
//	BOOL RxdTestLink();
//	BOOL RxdReqLinkStatus();//������·״̬
//	BOOL RxdClass1Data();//һ������
//	BOOL RxdClass2Data();//��������
//	
//	BOOL RxdSetPara();
//	BOOL RxdSetPara2();
//	BOOL RxdSetPara3();
//	BOOL RxdClockSyn();
//	BOOL RxdCallAll();
//	BOOL RxdCallCI();//�ٻ�������
//
//	BOOL RxdNoAsdu();
//	BOOL RxdDelay();
//	BOOL ResetProcess();
//
//	BOOL RxdYkCmd();
//	BOOL RxdYtCmd();
//	BOOL RxdTqCmd();
//	BOOL RxdDOCmd();
//
//	BOOL RxdReadRequestFile();
//	BOOL RxdWriteRequestFile();
//	BOOL RxdReadFileData();
//	BOOL RxdWriteFileData();
//
/////file function
//	BOOL RxdCallDirFile();
//	BOOL TxdCallDir();
//	BOOL TxdFileSelect();
//	BOOL TxdFileRequest();
//
//	
//	BOOL SearchClass1();
//	BOOL Txd_Class1();
//	BOOL Txd_Class2();
//	BOOL TxdCallAll();
//	BOOL TxdCallAIData();
//	BOOL TxdCallAIData13();
//	BOOL TxdCallBIData();
//	BOOL TxdCallDBIData();
//	BOOL TxdCallCIData();
//	BOOL TxdCallGroupCIData(WORD wGroup);
//
//	BOOL TxdScanAiData();
//	BOOL TxdScanAiData13();
//
//	BOOL TxdCallStepData();
//	BOOL TxdCallBcdData();
//
//	BOOL TxdChangeSpiData();
//	BOOL TxdChangeSpiData32();
//
//	BOOL TxdFreezeCIData();
//
//	BOOL TxdCINoTimeData();
//	BOOL TxdCICP24Time2a();
//	BOOL TxdCICP56Time2a();
//	BOOL TxdChangeBcdData();
//	
//	BOOL TxdCIReadTermination();
//	BOOL TxdMirror();
//	
//	BOOL Txd_ChangeAI();
//	BOOL Txd_ChangeAI13(); //���ͱ仯ң�⸡������
//	BOOL Txd_ChangeAI14();
//	BOOL Txd_ChangeAI36();//���ͱ仯ң�ⳤʱ�긡������
//	BOOL Txd_ChangeAI34();//���ͱ仯ң�ⳤʱ��normalisee����
//
//	BOOL TxdTestLink();
//	BOOL TxdSetPara();
//
//	BOOL Txd_BurstBI();
//	BOOL Txd_BurstDBI();
//	
//	BOOL Txd_Soe();
//	BOOL Txd_CP56TSoe();
//	BOOL Txd_CP24TSoe();
//
//	BOOL Txd_DSoe();
//	BOOL Txd_CP56TDSoe();
//	BOOL Txd_CP24TDSoe();
//	
//	BOOL Txd_ProtectMr();
//	BOOL Txd_Protect38(TSysTimeMr* pSysMr);
//	BOOL Txd_Protect39(TSysTimeMr* pSysMr);
//	BOOL Txd_Protect40(TSysTimeMr* pSysMr);
//
//	BOOL Txd_YcSoe();
//	BOOL Txd_YcSoe34();
//	BOOL Txd_YcSoe36();
//
//	BOOL TxdClockSyn();
//	BOOL TxdTimeDelay();
//
//	BOOL Txd_AllStop();
//	BOOL TxdCallAllAck();
//	BOOL TxdInitFinish();
//	BOOL TxdYtReturn();
//	BOOL TxdTqReturn();
//	BOOL TxdYkReturn();
//	BOOL TxdDOReturn();
//		
//	BOOL TxdYkExeInfo(BYTE bReason);
//	BOOL TxdYtExeInfo(BYTE bReason);
//
//
//	BOOL Txd_NoData();
//	BOOL ACK();
//	BOOL NACK();
//	BOOL Txd_NoData_E5();
//
//	BOOL TxdFixFrame(BYTE bPRM, BYTE bCode);
//
//	BOOL TxdFileStatus();
//
//	//���ͱ��Ĵ�����
//	BYTE GetCtrCode(BYTE PRM,BYTE dwCode); //OK
//	BOOL Txd_Head(BYTE dwCode,BYTE Reason);//���ͱ���ͷ
//	BOOL Txd_Tail(BYTE PRM,BYTE dwCode,BYTE Num);//���ͱ���β
//
//	BOOL GetFileStatus(char* pName, TFileStatus* pFileStatus);
//	WORD GetFileCrc16(char* pFileName);
//	WORD Crc16(WORD wCrc, BYTE bData);
//	DWORD Standard_C_Float_To_Dword(DWORD C_Float,int DecimalPoint);
//
//	void TestMr();
//
//	virtual void OnBurstBI();
//	virtual void OnSoe();
//
//};

#endif

