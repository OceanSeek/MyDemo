#ifndef __DBASE_H__
#define __DBASE_H__


#define MAX_FILE_LEN			(256*1024)  //���ȴ��ڸ������ļ������ܱ����� 
#define MAX_DEVEICE_NUM			150			//ϵͳ�������õ����豸�����������߼��豸��ʵ���豸������ 
#define MAX_ROW 					200			//ÿ�ζ�ȡ�������������
#define MT_USER    0x26c   //ʵ�ʿ�
#define MT_LOGIC   0x276   //�߼���
//=============  ͨѶ���λ����  ===============
#define FM_SHIELD   	0xFFFF0000      //������
#define FM_OK       	0x00010000      //��⵽һ��������֡
#define FM_ERR      	0x00020000      //��⵽һ��У������֡
#define FM_LESS     	0x00030000      //��⵽һ����������֡����δ���룩
//ң����Ϣ
#define PBOINFOSIZE  	8
//ң����Ϣ
#define PAOINFOSIZE  	8
//ͬ����Ϣ
#define PTQINFOSIZE  	8

#define TP_AOSELECT    		1    //ң��Ԥ��
#define TP_AOOPERATE   		2    //ң��ִ��
#define TP_AOCANCEL    		3    //ң������
#define TP_AODIRECT    		4    //ң��ֱ��ִ��
#define MAX_FLAGNO    			256   	//ÿ��ģ�����ı�־λ��
#define MAX_DWORDFLAGNO			32		//ÿ��ģ������DWORD��־����

#define SM_P(PVID) {}
#define SM_V(PVID) {}
  //========================================/|
 //PV����								   /||
//========================================/ ||
#define MPV_BIBURST  	0    //ң�ű�λר��PV	||  ||
#define MPV_SOE      	1    //SOEר��PV	||  ||
#define MPV_GVARS    	2    //gVarsר��PV	||  ||
#define MPV_OPENCOMM 	3    //����ר��PV	||  ||
#define MPV_LOGERR   	4    //LOGERRר��PV	||  ||
#define MPV_TIME     	5    //TIMEר��PV	||  ||
#define MPV_MEM      	6    //MEMר��PV		||  ||
#define MPV_OI       	7    //OIר��PV		||  
#define MPV_TASK    	8	 //����	
#define MPV_DBIBURST  	9    //˫��ң�ű�λר��PV	||
#define MPV_DBISOE      10   //˫��SOEר��PV	||  ||
#define MPV_MR 	     	11   //��ר��PV	||
#define MPV_AIT 	    12   //��ר��PV	||
#define MPV_NETDRV		13	//��̫��������
#define MPV_PROMPT		14	//��ʾ��Ϣ��
#define MPV_QSPI		15
#define MPV_OPINFO		16



uint8_t ChkSum(uint8_t *buf, int len);
uint8_t GetDevFlag(int DevID,int DevNo);
void ClearDevFlag(int DevID,int DevNo);
BOOL SetDevFlag(int DevID, int DevNo);
void SetDevFlagValue(int    DevID, DWORD dwFlagNo, DWORD dwValue);
int GetDevNo(int DevID);
int CheckBrustDZ(void);
int CheckBrustYxSoe(void);
int CheckBrustYx(void);


enum {
    DB_ID,			
    DB_Name,		
    DB_Address,		
    DB_IP,			
    DB_PORT,		
    DB_bault,		
    DB_DataBit,			
	DB_StopBit,		
	DB_Parity,		
	DB_DevType, 		
	DB_ProtocolType, 	
	DB_InterfaceType,
	DB_SerialID,		
	DB_NetID, 		
	DB_TcpType, 	
};

enum {
    WQ900_ID,			
    WQ900_ParentID,		
    WQ900_Address,		
    WQ900_YCStart,		
    WQ900_BIStart,	
	WQ900_DZStart,	
    WQ900_DBIStart,	
    WQ900_YKStart,		
    WQ900_YMStart,		
    WQ900_AINum,		
    WQ900_BINum,		
	WQ900_CINum,		
	WQ900_BONum,		
	WQ900_AONum,		
    WQ900_DZNum,		
};

enum {
    PLC_ID,			
    PLC_ParentID,		
    PLC_Address,		
    PLC_CoiStatusStart,		
    PLC_InputStatusStart,		
    PLC_HoldingRegStart,	
    PLC_InputRegStart,		
    PLC_CoiStatusNum,		
    PLC_InputStatusNum,		
    PLC_HoldingRegNum,		
	PLC_InputRegNum,
	PLC_DestIP,
	PLC_DestPort,
};



void InitDbase(void);



//��Ҫ��0��ȫ�ֱ�������

typedef struct	//I104S��Լ�Ŀɱ�֡���ṹ  
{
    unsigned char Start; 		//�����ַ�
    unsigned char Length; 		//����
    unsigned char Control1;		 //������1
    unsigned char Control2; 	//������2
    unsigned char Control3; 	//������3
    unsigned char Control4; 	//������4
    unsigned char Style;   		//���ͱ�ʶ��7
    unsigned char Definitive; 	//�ṹ�޶���
    unsigned char Reason_L; 	//����ԭ��
    unsigned char Reason_H; 	//����ԭ��
    unsigned char Address_L; 	//������ַ����վվַ��
    unsigned char Address_H; 	//������ַ����վվַ��
    unsigned char AddInfom1;  	//��Ϣ���ַ����13�ֽ�
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
    
}TS104APDU;


typedef struct{
	uint32_t dwTaskNum; //��������
	uint32_t dwDevNum; //ϵͳ�����豸���� 
	uint32_t dwPortNum; //ϵͳ���Ӷ˿�����

//	SEM_ID adwPVID[MAX_PVNUM+1]; //PV����ID
	uint32_t dwVarNum; //��鿴�����ĸ���
//	TVarInfo VarInfo[MAX_VAR_NUM+1];
//�����ж������Ƿ��Ѽ���

//���ڱ����ظ�����/������Ϣ	
//	uint32_t adwLogged[MAX_LOGGED_NUM+1];
	uint32_t dwTID_BI;
	uint32_t dwTID_JC;
	uint32_t dwTID_AO;
	uint32_t dwTID_TQ;
	uint32_t dwTID_BO;
	uint32_t dwTID_BCD;
	uint32_t dwTID_DCAI;
	uint32_t dwTID_PRINT;
	uint32_t dwAddress;//1725���ɼ�ģ��ʱ�ĵ�ַ
	uint32_t dwBHType; //����ϵͳģ��ı������ͱ�ʶ����Һ����ʾ��
	int8_t bTimeSyn;//ʱ��ͬ��״̬��	
//����
	
	uint32_t dwTID_KEYBOARD;  //KEYBOARD����ID
	uint32_t dwTID_DISPLAY;   //DISPLAY����ID
	uint32_t dwTID_WatchBakPort;//�����ڼ�������
	
	uint32_t dwTID_EtherNetDrv; //��̫����������
	uint32_t dwTID_SerialDrv;   //������������
	uint32_t dwTID_MFdkBusDrv;  //FDKBUS��վ����������
	uint32_t dwTID_SFdkBusDrv;  //FDKBUS��վ����������
	uint32_t dwTID_MCanBusDrv;  //CANBUS��վ����������
	uint32_t dwTID_SCanBusDrv;  //CANBUS��վ����������

	void *pFirstMalloc; //��̬�����ڴ����ʼ����
	void *pLastMalloc; //��̬�����ڴ�Ľ�������

	void *pMemFree;   //��̬�ͷ��ڴ����ʼ����

//СƬ�ڴ涯̬������ƹ���
	int8_t *pLittleRam; //СƬ�ڴ�δ������ʼƫ����δ������ʼƫ����
	uint32_t dwLittleRamSize; //СƬ�ڴ�δ������ʼƫ����

//NVRAM
	uint32_t dwNvRamPtr; //NVRamδ������ʼƫ����
	uint32_t dwNvRamID; //NVRam��������ID
	
//	TOIEWrap OIEWrap; //͸��������
//	TCommBlock CommBlock; //ͨѶ���ļ���
//	TBIEvent BIEventInfo;//��¼��Ҫ��ң�ű�λ��SOE������
//	TBOControl BOControlMsg; 
//	TMREvent MREventInfo;	//��¼��Ҫ�����¹ʼ򱨵�����

	uint32_t TransYXTableNum;
	uint32_t TransYCTableNum;
	uint32_t TransYKTableNum;
	uint32_t TransDZTableNum;
	uint32_t TransModbusCoidStatusTableNum;
	uint32_t TransModbusInputStatusTableNum;
	uint32_t TransModbusHoldingRegTableNum;
	uint32_t TransModbusInputRegTableNum;
	
	uint32_t YxSoeNum;
	uint32_t YcSoeNum;
	uint32_t YxWritePtr;//ң��дָ��
	uint32_t DYxWritePtr;//˫��ң��дָ��
	uint32_t DZWritePtr;//��ֵдָ��
	uint32_t YxSoeWritePtr;//ң��дָ��
	uint32_t YxDSoeWritePtr;//˫��ң��дָ��
	uint32_t AitWritePtr;//��ʱ��ң��дָ��
	uint32_t BCDWritePtr;
	uint32_t MrWritePtr;//�¹ʼ�дָ��
	uint32_t YcWritePtr;//ң��дָ��
	uint8_t	Flag_ReSet;
}TVars;


typedef struct {
	uint32_t	value;		//32λ��ֵ
	uint16_t	nNo;		//ת�����
	uint8_t		flag;		//
	uint8_t		flag_call;	//���ٻ���־
	uint32_t	YZ_value;	//Ԥ��ֵ
	int			YZ_Caller;	//Ԥ����Դ���豸ID��

	/*5512-VXWORKS*/
	DWORD dwNo;//ת�����
	WORD  wStatus;
	DWORD sValue; //
	/*5512-VXWORKS*/
}TBurstDZ;


#pragma pack(1)	//��1�ֽڶ���


//��׼ָ������ṹ
typedef struct{
	uint32_t dwWritePtr; //дָ��
	uint32_t dwReadPtr;  //��ָ��
}TPtr;

typedef struct{			//�����ڲ�����RAM��
	TPtr Ptr;
}TSoeWrap;

/*----------------------------------------------*/
/* �߼��豸��ֵ�����ⶨ��                                                                 */
/*----------------------------------------------*/
typedef struct{                    //modify by lgh
    WORD        wRealID;                /*�����豸ID*/
    WORD        wOffset;                /*��ֵ��ʵʱ���е�ƫ����*/
    short	a;			 //ϵ��A
	short	b;			 //ϵ��B
	WORD	wFlag;
}TLogicDZ;


/*----------------------------------------------------------------------------------------*/
/* �������ʱ���ʽ                                                                          */
/*----------------------------------------------------------------------------------------*/
typedef struct {

	uint16_t MSecond;				 /*����*/
	uint8_t Minute; 				/*��*/
	uint8_t Hour;					/*ʱ*/
	uint8_t Day;					/*��*/
	uint8_t Month;					/*��*/
	uint8_t Year;					/*��*/
}TSysTime,*PTSysTime;


typedef struct {
	uint16_t  			nNo;			//���
	uint8_t  			bStatus;		//״̬ 0:�ɺ�����   ��0���ɷ�����
	uint8_t  			bType;       	//0x0A-��  0x0B-������� 0x0C-��ѹ 0x0D-Ƶ�� 0x0E-����
	IEC10X_DETECT_T		detect16;//16λң��ֵ
	IEC10X_DETECT_DW_T 	detect32;//32λң��ֵ
  	CP56Time2a_T stTime;    	//�ꡢ�¡��ա�ʱ���֡��롢����
}TSysTimeSoe,*PTSysTimeSoe;

typedef struct {
	WORD 	 nNo;			//���
	BYTE	 bStatus;		//״̬ 0x01:��; 0x02:��; 0x00:��������;	0x03���Ƿ�
	CP56Time2a_T stTime;    	//�ꡢ�¡��ա�ʱ���֡��롢����
}TSysTimeDBISoe;

typedef struct {
	WORD 	 nNo;			//���
	BYTE	 bStatus;		//״̬ 0x01:��; 0x02:��; 0x00:��������;	0x03���Ƿ�
	TSysTime stTime;    	//�ꡢ�¡��ա�ʱ���֡��롢����
}TAbsTimeDBISoe;



typedef struct {
    WORD       Lo;                     /*��λϵͳʱ��*/                    
    DWORD        Hi;                     /*��λϵͳʱ��*/
}TAbsTime;

typedef struct {
	WORD		nNo;		//���
	BYTE		bStatus;	//״̬ 0:�ɺ�����   ��0���ɷ�����
	TAbsTime	stTime;		//����ʱ��
}TAbsTimeSoe;


typedef struct {
	WORD 	 	nNo;		//ң����
	short	 	sValue;		//Խ��ң��ֵ
	TAbsTime	stTime;		//����ʱ��
}TAbsTimeAit;


typedef struct {
//��ǰ�����������Ϣ
	WORD wFlag;	  		//����״̬  PBO_NULL, PBO_BUSY, PBO_WAITOPERATE
	DWORD dwAppID;    	//�·�ң�ص�����ID;
	DWORD dwNo;       	//ʵʱ��·�ţ����
	DWORD dwAttrib;   	//���� //״̬���Ϸ֣�TP_NULL TP_OPEN, TP_CLOSE,
	DWORD dwCommand;  	//���� TP_YKSELECT TP_YKOPERATE  TP_YKCANCEL TP_YKDIRECT
	DWORD dwResult;   	//���ķ��ص�ң�ط�У״̬ PBO_SUCESS  PBO_FAIL
	DWORD dwRetNo;    	//�·���·�ţ����
	DWORD dwRetDevID;	//�·�ң�ص�device ID
	DWORD dwQualitier;	//����������޶� ���ǵ������ԣ���������
	DWORD dwExeCount;    	//ң��ִ�д���
	DWORD dwCTime;		// ����ʱ��s
	DWORD dwTimeH;		//�̵����պ�ʱ�䣬
	DWORD dwTimeL;		//�̵�����ʱ�䣬

	BYTE  Info[8];
}TBO;

/*----------------------------------------------------------------------------------------*/
/* ʵ���豸ң�������ⶨ��                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char	   szName[20];	/*ң�ض�������*/
    DWORD      dwBOID;      //��16λΪ����  0:��  1��CLOSE �� 2��Open��
    			        	//��16λΪ���غ�/·��
	BYTE		ucType;		//�������ͣ��ֶ���Ϊ����5�֣�
							//0��NUL������
							//1������ON��������ѭ�����ON-OFF����󱣳���OFF̬��
							//2������OFF��������ѭ�����OFF-ON����󱣳���ON̬��
							//3��˨��ON���������ON̬��
							//4��˨��OFF���������OFF̬��
							//5~255������
							//ע�����������ʽ�е����ڣ��������wOnTime��wOffTime
							//ָ����
	BYTE		ucCount;	//���������ʽ�е����������
	WORD		wOffTime;	//���������ʽ�е�OFF̬����ʱ����ms��Ϊ��ԭ�������ݣ�
							//���԰�wOffTime������ǰ��
	BYTE 		ucAutoResetTime;	//�Զ�����ʱ��(s)
	WORD	   	wLastTime;	/*�̵�����������ʱ��*/
}TRealBO;


//�¹ʼ�
typedef struct { //malfunction report
	WORD 	 	wAddr;			//����ģ���ַ
	DWORD 	 	dwBhType;		//��������
	WORD 	 	wAttribute;		//�����ԣ�����/����
	WORD 	 	wMrNo;			//�����
	DWORD		dwTripType;		//��բ����
								//bit0--I������
								//bit1--II������
								//bit2--III������
								//bit3--��������
								//bit4--����������
								//bit5--���������
								//bit6--�غ�բ��
								//bit7--����˹����
								//bit8--380V���򱣻�����
								//bit9--����ѹ����
								//bit10--�͵�ѹ����
								//bit11--��ƽ���ѹ����
								//bit12--��ƽ���������

	TSysTime 	stTime;     	//������ʼʱ�� �ꡢ�¡��ա�ʱ���֡��롢����
	DWORD		dwTripTime;		//����ʱ�� ���ϵͳʱ��ĺ�����
	DWORD		dwTwjTime;		//TWJʱ�� ���ϵͳʱ��ĺ�����
	DWORD		dwChzTime;		//�غ�բʱ�� ���ϵͳʱ��ĺ�����
	DWORD		dwHwjTime;		//TWJʱ�� ���ϵͳʱ��ĺ�����
	WORD		wRsv[4];		//������

	WORD		wDataNum;		//����ֵ������
//	short		sData[16];		//����ֵ���ݣ�
	DWORD		dwData[16];		//����ֵ���ݣ�
}TSysTimeMr;


//--------------------------------------
// ��λң�������ⶨ��						
//--------------------------------------
typedef struct {
	IEC10X_DETECT_F_T	detect32_F;//32λ����ң��ֵ
	IEC10X_DETECT_DW_T	detect32;//32λң��ֵ
	IEC10X_DETECT_T 	detect16;//16λң��ֵ	
	uint16_t			nNo;			//���
	uint16_t			nIndex;				//������
	uint8_t 			bType;			//2两字节，4字节
	uint8_t			flag;			//0:�� 1:ͻ����־ 2:���ٻ���־
//	TSysTime stTime;		//�ꡢ�¡��ա�ʱ���֡��롢����
	/*5512-VXWORKS*/
	DWORD dwNo;//ת�����
	WORD  wStatus;
	DWORD sValue; //
	TSysTime stTime;
	/*5512-VXWORKS*/
}TBurstAI;


//--------------------------------------
// ��λң�������ⶨ��						
//--------------------------------------
typedef struct {
	CP56Time2a_T 	stTime;        			//���ݿ��ڼ�⵽��λ���Զ����ɵ�ϵͳʱ��
	uint16_t		nNo;					//���ͺ�
	uint16_t		ID;						//������
	uint8_t			bStatus;				//״̬
	uint8_t		flag;					//0:�� 1:ͻ����־ 2:���ٻ���־
	uint8_t			FlagSoe;				//0:�� 1:ң���¼�ͻ����־
}TBurstBI,*PTBurstBI;

//--------------------------------------
// ��λң�ż�¼						
//--------------------------------------
typedef struct {
	CP56Time2a_T stTime;
	uint8_t		bStatus;			//״̬
	uint16_t	point;				//���͵��
}TBurstYXRecord,*PTBurstYXRecord;

typedef struct {
	DWORD dwNo;
	WORD wStatus;
	DWORD dwValue;
	TSysTime stTime;
}TBurstBcd;


//--------------------------------------
// ��λң�ż�¼						
//--------------------------------------
typedef struct {
	CP56Time2a_T stTime;
	uint32_t	Value32;//32λң��ֵ
	uint16_t	point;				//���͵��
}TBurstYCRecord,*PTBurstYCRecord;

//--------------------------------------
// ��λң�Ż��ͻ���������													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//дָ��
}TBurstAIWrap;

//--------------------------------------
// ��λң�Ż��ͻ���������													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//дָ��
}TBurstBIWrap;

//--------------------------------------
// ��λң�Ż��ͻ���������													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//дָ��
}TBurstDBIWrap;

//--------------------------------------
// ��λң�������ⶨ��						
//--------------------------------------
typedef struct {
	uint32_t		nNo;				//���ͺ�
	uint8_t		bStatus;				//״̬
	CP56Time2a_T 	stTime;        			//���ݿ��ڼ�⵽��λ���Զ����ɵ�ϵͳʱ��
}TBurstDBI;

//--------------------------------------
// ��λң��SOE�����ⶨ��						
//--------------------------------------
typedef struct {
	uint8_t		nFlag;					//�豸��
	uint8_t		bStatus;				//״̬
	uint8_t		bResdStatus;			//��ȡ״̬,0��δ����1���Ѷ���
	uint32_t	nPoint;					//�豸�ڵ��
	uint32_t	nIndex;					//ת�����
	TSysTime 	stTime;        			//�洢��λʱ��
}TBurstBISoe;

//--------------------------------------
// ��λң��SOE�����ⶨ��						
//--------------------------------------
typedef struct {
	uint8_t		nFlag;					//�豸��
	uint8_t		bStatus;				//״̬
	uint8_t		bResdStatus;			//��ȡ״̬,0��δ����1���Ѷ���
	uint16_t	bValue;					//�豸�ڵ��
	uint32_t	nPoint;					//�豸�ڵ��
	uint32_t	nIndex;					//ת�����
	TSysTime 	stTime;        			//�洢��λʱ��
}TBurstAISoe;



//--------------------------------------
// ת���ⶨ��						
//--------------------------------------
typedef struct {
	uint16_t		ID;					//������	
	uint16_t		wRealID;			//�����豸ID��
	uint16_t		nPoint;				//�豸�ڵ��
	uint16_t 		nIndex;        		//ת�����
	
}TransTable_T,*PTransTable_T;

typedef struct {
	uint16_t		ID;					//主键	
	uint16_t		wRealID;			//实际设备-ID号
	uint16_t		nPoint;				//实际设备-内点号
	uint16_t 		nIndex;        		//逻辑设备-发送点号
	uint16_t 		RegAddr;        	//实际设备-寄存器地址
	uint16_t 		SlaverAddr;        	//实际设备-从机地址
	uint8_t 		DataType;        	//实际设备-数据类型，2、4字节
	uint8_t			DataOffset;			//实际设备-偏移位数
}Modbus_TransTable_T,*PModbus_TransTable_T;


/*----------------------------------------------------------------------------------------*/
/* �߼��豸ң�������ⶨ��                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wConfig;            //�����ֽ�
    WORD        wRealID;            //�����õ�AI�����������豸��ID
    WORD        wOffset;            //�����õ�AI�����������豸��ʵ�ʿ��е�ƫ����
    short       a;                  //ң��ϵ��
    short       b;
    short       c;
    WORD 		    wSQZ;		   			    //����ֵ ��λ1��ʾ���������0,��������
    WORD		    wFlag;				      //���ݷ���ʱ������õ�
    DWORD		    sOldValue;			    //���ڱ仯ң��ļ�¼
    WORD		    wOldFlag;
    TSysTime	  stTime;
	short	type;//0x09,0x0B,0x0D;归一化，标度化，短浮点
//	short	sHigh;
//	short	sLow;
//	short	sHighBack;
//	short	sLowBack;
	short	sDeadBand;
}TLogicAI;





typedef struct{
	DWORD dwValue;
	WORD wStatus;
	TSysTime stTime;
}TFreezeCI;

typedef struct{
	DWORD dwBcdBurstReadPtr;//�仯bcd��ָ��
	DWORD dwBcdBurstWritePtr;//дָ��
	TBurstBcd		*pBurstBcd;
}TBurstBcdWrap;


/*----------------------------------------------------------------------------------------*/
/* �߼��豸ң�������ⶨ��                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wRealID;            //�����õ�AI�����������豸��ID
    WORD        wOffset;            //�����õ�AI�����������豸��ʵ�ʿ��е�ƫ����
    TSysTime	  stTime;
	WORD		wFlag;		/*��Լ��������ʱʹ��*/
}TLogicBI;

/*----------------------------------------------------------------------------------------*/
/* �߼��豸ң�������ⶨ��                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wRealID;                /*��BI���������豸��ID*/
    WORD        wOffset;                /*��BI�����������豸ʵʱ���е�ƫ����*/
    WORD        wConfig;                /*�����ֽ�*/
										/*D0=1 ���ù�̬���� =0 �����ù�̬����*/
										/*D1=1 �����¼����� =0 �������¼�����*/
										/*D2=1 ����ԭʼ���� =0 ������ԭʼ����*/
										/*D3=1 ����ת�����������Ϣ =0 ������������Ϣ*/
										/*D4=1 �¹��ܱ�־��Ч =0 ��Ч */
										/*D5~7����*/
	WORD	wFlag;						/*��Լ��������ʱʹ��*/
}TLogicDBI;



typedef struct{
    WORD	wRealID;                /*��DD�����豸��ID��*/
    WORD	wOffset;                /*��DD���豸ʵ�ʿ��е�ƫ����*/
    WORD	wNo;
    WORD	wConfig;                /*�����ֽ�*/
										/*  D0=1 Ϊ������ =0 Ϊ������*/
										/*  D1~7����*/
    short   a;
    short   b;
    short   c;
	BYTE	bFreezeFlag;			/*�����־��ͬʱ��ֹ����û�ж��߶�����*/
	BYTE	bReset;					/*bReset=1���Ͳ�ֵ, bReset=0��������*/
    WORD    wFreezeTime;			/*����ʱ��*/
	WORD	wCountTime;
	DWORD	dwTotalValue;			/**/
	WORD	wStatus;				/*����*/
	DWORD	dwFreezeValue;			/*���������*/
	WORD	wFlag;					/*���ͱ�־*/
	TSysTime stTime;				/*�����ʱ��*/
}TLogicCI;



typedef struct TREFLEX {
    DWORD       dwDevNo;                 /*���ø����ݵ��߼����豸���*/
    DWORD       dwNo;                    /*���������߼��豸���е�ƫ����*/
    struct TREFLEX *pNext;
}TReflex, *PTREFLEX;

typedef struct{
    WORD        wRealID;            /*��BCD���������豸��ID*/
    WORD        wOffset;            /*��BCD�����������豸ʵʱ���е�ƫ����*/
	WORD		wFlag;				/*��Լ��������ʱ��*/
}TLogicBCD;

typedef struct {
	WORD wFlag;	        //����״̬  PAO_NULL, PAO_BUSY, PAO_WAITOPERATE
	WORD wNo;			//ң��·��
	DWORD dwCommand;	//���� TP_AOSELECT TP_AOEXECUTE  TP_AOCANCEL TP_AODIRECT
	DWORD dwResult;
	DWORD dwDoValue;	//���������ֵ
	DWORD dwOnTime;		//on ʱ��
	DWORD dwOffTime;	//off ʱ��
	DWORD dwCount;	    //����
	WORD wNeedRetFlag;      //�Ƿ���Ҫ�����������У��Ϣ��־1:need,0:NoNeed
	BYTE bInfo[PAOINFOSIZE];//������ң�ڱ���������Ϣ
//	TDOMsg DOMsg;           //���������ԭ��Ϣ
	BYTE Info[8];
}TDO;

typedef struct {
	WORD wFlag;	        //����״̬  PAO_NULL, PAO_BUSY, PAO_WAITOPERATE
	WORD wNo;			//ң��·��
	DWORD dwCommand;	//���� TP_AOSELECT TP_AOEXECUTE  TP_AOCANCEL TP_AODIRECT
	DWORD dwResult;
	short sAoValue;	        //ң��ֵ
	WORD wNeedRetFlag;      //�Ƿ���Ҫң����У��Ϣ��־1:need,0:NoNeed
	BYTE bInfo[PAOINFOSIZE];//������ң�ڱ���������Ϣ
//	TAOMsg AOMsg;           //ң��ԭ��Ϣ
	BYTE  Info[16];
}TAO;

/*----------------------------------------------------------------------------------------*/
/* ʵ���豸ң�������ⶨ��                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char		szName[20];		/*YT��������*/
    WORD        wControl;               /*rsv*/
    WORD        nID;                    /*ң��������*/
	WORD		wMaxValue;              /*��ֵ*/		/*a,b,c �ֱ��Ӧϵ��������ֵ*/
	BYTE		bResetTime;
	BYTE		bRsv1;
	WORD		rsv2;
}TRealAO;


/*----------------------------------------------------------------------------------------*/
/* ʵ���豸ң�������ⶨ��                                                                     */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char		szName[20];				/* ң������*/
	DWORD		dwControl;				/*�����ֽ�*/
	WORD		wForceToZero;
	short   a;            /*a,b,cΪϵ��,�������y=x*a/b+c*/
	short   b;
	short   c;
	short   sHigh;        /* ���� */
	short   sLow;         /* ���� */
	short		sHighBack;				/* ң����ϱ���ֵ*/
	short		sLowBack;				  /*ң�����ֵ*/
	WORD		wDeadBand;				/* ң������ֵ ,D15=1�������ֵ,=0Ϊ��������ֵ */
	WORD		wSmoothFactor;		/*���϶���ʱ��*/
	WORD	  wType;					/*ֱ����������*/
	TReflex *pReflex;   	/*�߼������������*/
}TRealAI;


typedef struct {			//��ʱ��ң�����;������ram
	TPtr Ptr;
}TAitWrap;

//====================================
//	��ʱ��ң�⣨Խ��ң�⣩��ؽṹ
//====================================
typedef struct {
	uint16_t 	 	    nNo;		//ת�����
	uint16_t 	 	    ID;		//������
	uint8_t        bStatus;    //Խ��״̬0��Խ����1��Խ����2��Խ���޻ָ�3��Խ���޻ָ�
	uint16_t	 	    sValue;		//Խ��ң��ֵ
	uint32_t	 	    sValue32;		//Խ��ң��ֵ
	TSysTime 	  stTime;    	//�ꡢ�¡��ա�ʱ���֡��롢����
}TSysTimeAit;

//====================================
//		�߼�����ؽṹ
//====================================
typedef struct{
    DWORD 			    *pAddCI;			//��ȵ��ۼ�ֵ,�Ǽ���ֵ������RAM��
	DWORD			      *pLogicOldBcd;		/*����ľɵ�bcd��ֵ*/
    TLogicDZ        *pLogicDZ;          /*��ֵ�߼����Կ�ͷָ��,�����ڶ�̬RAM��*/    
    TLogicAI        *pLogicAI;          /*ң���߼����Կ�ָ��,�����ڶ�̬RAM��*/
    TLogicBI        *pLogicBI;          /*ң���߼����Կ�ָ��,�����ڶ�̬RAM��*/
    TLogicCI        *pLogicCI;          /*ң���߼����Կ�ָ��,�����ڶ�̬RAM��*/
    TLogicBCD       *pLogicBCD;         /*BCD�߼����Կ�ָ��,�����ڶ�̬RAM��*/
    TLogicDBI       *pLogicDBI;         /*ң���߼����Կ�ָ��,�����ڶ�̬RAM��*/
    TBurstAIWrap     BurstAIWrap;		/*�仯ң��ͷָ��,�����ڶ�̬RAM��*/
	TBurstBcdWrap burstBcdWrap;
}TLogicBase;

//--------------------------------------
// ʵ���豸ң�������ⶨ��                                                                 */
//--------------------------------------
typedef struct {
	char	szName[20];	/*ң�Ŷ�������*/
	DWORD 	dwControl;	//D0~15 ΪI/O��Ԫʹ�õĿ���λ; �����ذ��޹�
						//D16~31Ϊ����ʹ�õĿ���λ����I/O��Ԫ�޹�
						//D0 = 1 ��������ʱ����¼����ݣ� = 0 ������
						//D1 = 1 ������ʱ����¼����ݣ� = 0������
						//D2 = 1 ��һ��ȡ���� = 0 ��һ�㲻ȡ��
						//D3 = 1 ����   = 0 ������
						//D4 = 1 �¹�����Ч  = 0 ��Ч
						//D5 = 1 ���¹����ź�
						//D6 = 1 �ڶ���ȡ���� = 0 �ڶ��㲻ȡ��
						//D7~ D8 ����
						//D9~D15  chatter filter �ı仯���������Ϊ127��

						//D16 = 1 �����ز�������ʱ����¼����ݣ� = 0 ������
						//D17 = 1 �����ز�����ʱ����¼����ݣ� = 0������
						//D18 = 1 �����ؽ��е�һ��ȡ�������� = 0 ��ȡ��
						//D19 = 1 �����ؽ��еڶ���ȡ�������� = 0 ��ȡ��
						//D20~31 ����

	WORD 	wFilterTime;	//Debounce����ʱ�䣬Ҳ������С����Ч������ms
	WORD 	wDINo;			//˫���еĵ�һ��Ľ���λ����ţ���0���𣬱���Ϊż��
							//����˫���еĵڶ��㣬�����λ�ñ��������һ����ڵ�
							//��һ�㣬�������Բ�������ʾ��
	WORD 	wChatterFilterTime; 	//chatter filter �ļ��ʱ�䣬ms�����Ϊ60000ms
	WORD 	wDPSynTime;				//˫��ͬ��ʱ�䣬ms
	DWORD	dwChatterNum;			//�����仯����
	DWORD	dwLockoutNum;			//�ȶ����ڸ���
	WORD	wSoeEnable;  			//���ڵ�һ���յ�BIʱ��ֹ����SOE
	WORD	wCosEnable;
	TReflex *pReflex;   
}TRealDBI;
//--------------------------------------
// ʵ���豸ң�������ⶨ��                                                                 */
//--------------------------------------
typedef struct {
	char	     szName[20];			/*ң�Ŷ�������*/
	DWORD        dwControl;             	/*D0=1 �����ݿ��������������ʱ����¼����ݣ�=0 ������*/
						/*D1=1 �����ݿ������������ʱ����¼����ݣ�=0������*/
						/*D2=1 ����ȡ��������=0 ��ȡ��*/
						/*D3=1 ���¹����ź�*/
						/*D4~7 ��*/
						/*  =0 ����ң��*/
						/*  =1 ˫��ң��*/
	WORD	    wDINo;	  			/*���Ӧ�����ֲɼ����*/
	WORD	    wFilterTime;  			/*��������*/
	WORD	    wChatterFilterTime;			//chatter filter �ļ��ʱ�䣬10ms��λ
	DWORD	    dwChatterNum;			//�����仯����
	DWORD	    dwLockoutNum;			//�ȶ����ڸ���
	WORD		  wSoeEnable;  			//���ڵ�һ���յ�BIʱ��ֹ����SOE 
	WORD		  wCosEnable;
	TReflex   *pReflex;   
}TRealBI;
//--------------------------------------
// ʵ���豸��������ⶨ��
//--------------------------------------
typedef struct {
    char	szName[20];				/*�������*/
    WORD    wControl;      	/*��ȿ�����,D2=1:����,D2=0:�����ͣ�D3=1:��������,D3=0:���ֵ��*/
    WORD    a;              /*ϵ��*/
    WORD    b;              /*ϵ��*/
    DWORD   dwOrgValue;     /*��ֵ*/
    WORD	  wDINo;
    WORD	  wDPSynTime;		//˫��ͬ��ʱ�䣬ms������˫��������Ч
}TRealCI;


typedef struct{
	DWORD					 wIOControl;	    	  /*new! I/O��Ԫʹ�õĿ����ֽ�*/
	DWORD					 wSControl;	    	    /*��������ʹ�õĿ����ֽ�*/
//	TRealDI	       *pRealDI;	          /*ң������ͷָ�룬�����ڶ�̬RAM��*/
  	TRealAI        *pRealAI;            /*ң��ʵ�ʿ�����ָ��,�����ڶ�̬RAM��*/
	TRealBI        *pRealBI;            /*ң��ʵ�ʿ�����ͷָ��,�����ڶ�̬RAM��*/
	TRealCI        *pRealCI;            /*���ʵ�ʿ�����ͷָ��,�����ڶ�̬RAM��*/
// 	TRealBO        *pRealBO;            /*ң�����Կ�,�����ڶ�̬RAM��*/
//  	TRealAO        *pRealAO;            /*ң�����Կ�,�����ڶ�̬RAM��*/
//	TRealSync      *pRealSync;          /*ͬ�����Կ�,�����ڶ�̬RAM��*/
//  	TTransDataAttr *pTransAttr;         /*͸������������,�����ڶ�̬RAM��*/
	TRealDBI       *pRealDBI;           /*˫��ң��ʵ�ʿ�����ͷָ��,�����ڶ�̬RAM��*/
//  TRealDO        *pRealDO;            /*������������Կ�,�����ڶ�̬RAM��*/
//  TRealBCD       *pRealBCD;           /*BCDʵ�ʿ�����ָ��,�����ڶ�̬RAM��*/
//  TRealDZ        *pRealDZ;            /*��ֵ���Կ�,�����ڲ�����RAM��*/
//  TRealMR        *pRealMR;            /*�¹ʼ����Կ�,�����ڲ�����RAM��*/
//  TRealSELF      *pRealSELF;          /*�Լ���Ϣ���Կ�,�����ڲ�����RAM��*/
}TRealBase;

typedef struct{			//�����ڲ�����RAM��
	TPtr Ptr;
}TDBISoeWrap;

typedef struct  
{
	WORD wStartAddr;//��Ϣ����ʼ��ַ��Ĭ��ֵ��0
	BYTE bStatus; //bit0, bit1 =1������=0����������
	BYTE bStatus1;//=1
	BYTE bStatus2;//2��ʱ�꣬30��ʱ��
	BYTE bClass;//���ȼ�
}TData_t;

typedef struct   //��Լio 
{
	unsigned short BufSize;		//��������С
	unsigned short ReadPtr;		//��ָ��
	unsigned short OldReadPtr;  //��ָ��
	unsigned short WritePtr;	//дָ��
	WORD	Address;			//��ַ��(��Ҫ���ڷ���)
	DWORD	Flag;				//������(��Ҫ���ڷ���)
	unsigned char  *Buf;		//������
}TCommIO;

typedef struct{

    uint16_t	ID;
	uint16_t	ParentID;		//ģ��ID
    uint16_t   	Address;		//�Լ��ĵ�ַ
	uint16_t	AIStart;	//ң��ƫ�Ƶ�ַ
	uint16_t	BIStart;	//ң��ƫ�Ƶ�ַ
	uint16_t	DZStart;	//��ֵƫ�Ƶ�ַ
	uint16_t	DBIStart;	
	uint16_t	BOStart;
	uint16_t	YMStart;
	uint16_t 	AINum;			//ң�����
	uint16_t	BINum;			//ң�Ÿ���
	uint16_t 	dwCINum;		//��ȸ���
	uint16_t	BONum;			//ң�ظ���
	uint16_t	AONum;			//ң������
	uint16_t	DZNum;			//��ֵ����
	uint16_t	SyncNum;		//ͬ�ڸ���
  	uint16_t 	TransNum;   	//͸�����ݹؼ��ָ���
	uint16_t 	DONum;			//�������������
	uint16_t 	dwBCDNum;			//BCD����
	uint16_t 	DBINum;			//˫��ң�Ÿ���

//	/*MODBUS*/
//	RTULocalMasterType *pModbusMaster;
//	ModbusData_T ModbusData;
}TWQ900,*PTWQ900;

typedef struct{

    uint16_t	ID;
	uint16_t	ParentID;		//ģ��ID
    uint16_t   	Address;		//�Լ��ĵ�ַ
	uint16_t	CoiStatusStart;	
	uint16_t	InputStatusStart;	
	uint16_t	HoldingRegStart;	
	uint16_t	InputRegStart;
	uint8_t		CoiStatusNum;
	uint8_t 	InputStatusNum;			
	uint8_t		HoldingRegNum;			
	uint8_t 	InputRegNum;	
	char		DestIP[32];
	uint16_t	DestPort;

//	/*MODBUS*/
//	RTULocalMasterType *pModbusMaster;
//	ModbusData_T ModbusData;
}TPLC,*PTPLC;



typedef struct 
{
	WORD wProtocolID;
	BYTE bBalance;//����
	BYTE bLinkAddr;//��·���ַ
	BYTE bAsduAddr;//���õ�ַ
	BYTE bInfoAddr;//��Ϣ���ַ
	BYTE bCauseTrans;//����ԭ��
	BYTE bTestMode;//����ģʽ,bit7:1:����ģʽ��0������ģʽ
	BYTE bCounterMode;// bit0,bit1 countermode, 0,1,2Modea, Modeb, modec
	WORD wCylicDataNum;//�������ݸ���,Ĭ����0,ң�����������
	WORD wScanTime;//ɨ��ʱ��
	WORD wMaxFrm;
	WORD wRsv[9];
	TData_t spi;
	TData_t dpi;
	TData_t step;
	TData_t bcd;//bit32string
	TData_t yc;
	TData_t kwh;
	TData_t yk;
	TData_t yt;
	TData_t bdo;//���������
	TData_t tq;
	TData_t mr;//����
	TData_t file;//�ļ�
	TData_t rsv[8];
	//	struct TData_t data [10];	//14�����ݽṹ���������4��Ϊ����
	WORD wCheckSum;	//�ۼӺͣ�ʹ�õ�16λ�������ǽ�λ
}TIEC101Cfg;


typedef struct{

//============ ���� ============//

    int16_t (*ReadAI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus); 	//��һ��ң��
    int16_t (*ReadAI32)(uint32_t dwDevID,uint32_t dwAINo, IEC10X_DETECT_F_T* pValue); 	//��һ��ң��
    void  (*WriteAI)(uint32_t dwDevID,uint32_t dwAINo,int16_t sValue);       	//дһ��ң��

    uint8_t  (*ReadDBI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//��һ��˫��ң��
    void  (*WriteDBI)(uint32_t dwDevID,uint32_t dwAINo,uint8_t bValue);        	//дһ��˫��ң��

    uint8_t  (*ReadBI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//��һ��ң��
    void  (*WriteBI)(uint32_t dwDevID,uint32_t dwAINo,uint8_t bValue,uint8_t bType,uint8_t bValue0,uint8_t bValue1,uint8_t bValue2,uint8_t bValue3);      //дһ��ң��	

    uint32_t (*ReadCI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);    //��һ�����
    void  (*WriteCI)(uint32_t dwDevID,uint32_t dwAINo,uint32_t dwValue);        //дһ�����

    uint32_t (*ReadBCD)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//��һ��BCD
    void  (*WriteBCD)(uint32_t dwDevID,uint32_t dwAINo,uint32_t dwValue);       //дһ��BCD

    uint32_t (*ReadDZ)(uint32_t dwDevID,uint32_t dwDZINo, uint16_t* pStatus); 	//��һ����ֵ
    void  (*WriteDZ)(uint32_t dwDevID,uint32_t dwDZNo,uint32_t sValue,uint16_t pStatus); //дһ����ֵ

    int (*SetYK)(int DevID, uint8_t Type, uint16_t COT, uint8_t YKData); //����ң����Ϣ
    int (*Receive)(int DevID, uint8_t *buf, uint16_t len); //��������
	int	(*Task)(int DevID);//ѭ������
	int	(*OnTimeOut)(int DevID);//��ʱ����
	int	(*SetDZ)(int DevID);//��ֵ����
	
    
    uint16_t	ID;
	uint16_t	DeviceID;		//ģ��ID
    char		Name[64];		//ģ������
    uint16_t   	SAddress;		//�Լ��ĵ�ַ
    uint16_t   	MAddress;		//�Է��ĵ�ַ
	char*	    SelfIP;			//�Լ���IP��ַ
	char*	    DirfIP;			//�豸��IP��ַ
	uint16_t 	AINum;			//ң�����
	uint16_t	BINum;			//ң�Ÿ���
	uint16_t 	dwCINum;		//��ȸ���
	uint16_t	BONum;			//ң�ظ���
//	uint16_t	AONum;			//ң������
	uint16_t	DZNum;			//��ֵ����
	uint16_t	SyncNum;		//ͬ�ڸ���
  	uint16_t 	TransNum;   	//͸�����ݹؼ��ָ���
	uint16_t 	DONum;			//�������������
	uint16_t 	dwBCDNum;			//BCD����
	uint16_t 	YBNum;			//��ֵ����
    uint16_t 	MRDeviceNum;	//�����¹ʼ򱨵��豸����ʵ�ʿ�̶�Ϊ1���߼����ѡ
	uint16_t 	DBINum;			//˫��ң�Ÿ���
	uint16_t 	SELFNum;		//�Լ���Ϣ�豸����
	uint16_t 	MaxSoeBuffer;	// ����soe/�仯ң��������
	uint16_t 	MaxDBISoeBuffer;// ˫��ң��soe/�仯ң��������
	uint16_t 	MaxAitBuffer;	// ����soe/�仯ң��������
	uint16_t 	bStarFlg;	// 
	uint32_t 	TimeCnt;	// 
	uint8_t 	Call_Time_cnt;	//
    uint16_t   	ParentID;
    uint16_t    KeyFlag;
    uint16_t    Address;
    uint16_t    AIStart;
    uint16_t	BIStart;
    uint16_t	DBIStart;
    uint16_t	DZStart;
    uint16_t    BOStart;
    uint16_t    YMStart;
	char 		Com[32];		//�ӿ�����:���ڻ򴮿�
	uint32_t 	SerialID;		//���ڱ��
	uint32_t 	NetID;			//���ڱ��
	char		TcpType[32];		//tcp����:client or server
	char 		Protocol[32];		//Э������
	char		Type[32];		//�豸����:�ն˻���վ
	char 		dwType;//�豸����:�ն˻���վ5512
	uint32_t	dwControl;
//    uint16_t    YCNum;			//ң�����
//    uint16_t    YXNum;			//ң�Ÿ���
//    uint16_t    YKNum;			//ң�ظ���
    uint16_t	YMNum;			//ң������
    char		IP[32];
    uint16_t    PORT;			//IP�˿ں�
    char		UDP_Dest_IP[32];
    uint16_t    UDP_Dest_PORT;			
	uint32_t	SerialBault;
	uint32_t	SerialDataBit;
	uint32_t	SerialStopBit;
	uint16_t	SerialParity;
	YK_INFO_T 	YKInfo;
	uint8_t 	*pSendBuf;//�������ݻ�����
	uint8_t 	*pRecBuf;//�������ݻ�����
	
	//ʵ�ʿ����߼��ⶼ�е�����
	TSoeWrap		*pSoeWrap;			//SOEָ�� NVRAM
	TSysTimeSoe 	*pSysTimeSoe;		//SOE�� NVRAM
	
	TRealBase     	*pRealBase; 
    TBurstAIWrap    *pBurstAIWrap;      /*ң�ű�λָ��,�����ڲ�����RAM��*/
    TBurstAI    	*pBurstAI;       	/*ң���λ��,�����ڲ�����RAM��*/
	
    TBurstBIWrap    *pBurstBIWrap;      /*ң�ű�λָ��,�����ڲ�����RAM��*/
    TBurstBI    	*pBurstBI;       	/*ң�ű�λ��,�����ڲ�����RAM��*/

    TBurstDBIWrap   *pBurstDBIWrap;     /*ң�ű�λָ��,�����ڲ�����RAM��*/
    TBurstDBI    	*pBurstDBI;       	/*ң�ű�λ��,�����ڲ�����RAM��*/

    TBurstDZ    	*pBurstDZ;       	/*��ֵ��,�����ڲ�����RAM��*/


    TAitWrap 		*pAitWrap;			//��ʱ��ң��ָ�� NVRAM
    TSysTimeAit 	*pSysTimeAit;		//��ʱ��ң��� NVRAM

    TDBISoeWrap 	*pDBISoeWrap;		//DOUBLE POINT YX SOEָ�� NVRAM
    TSysTimeDBISoe 	*pSysTimeDBISoe;	//DOUBLE POINT YX SOE�� NVRAM

    DWORD 		*pAI;               /*ң��ʵʱ��ָ��,�����ڲ�����RAM��,�߼���ָ���������ͨRAM*/
	uint16_t		*pAIStatus;			/*ң��ֵ��״̬*/
    uint16_t 		*pBI;               /*ң��ʵ�����ݿ�ͷָ��,�����ڲ�����RAM��*/
    uint16_t 		*pDBI;              /*ң��ʵ�����ݿ�ͷָ��,�����ڲ�����RAM��*/

    uint32_t 		*pCI;               /*��ȿ�ͷָ��,�����ڲ�����RAM��*/
	uint16_t		*pCIStatus;			/*�������*/
	
    uint32_t 		*pBCD;              /*BCDͷָ��,�����ڲ�����RAM��*/
	uint16_t		*pBCDStatus;		/*bcd status ptr*/
	
	uint16_t		*pBO;				/*ң��ֵ*/
	uint16_t		*pBOStatus;			/*ң������*/
	uint16_t		*pAO;				/*ң��ֵ*/
	uint16_t		*pAOStatus;			/*ң������*/
	uint32_t 		*pDZ;               /*��ֵ��λ��,�����ڲ�����RAM��*/
	uint16_t		*pDZStatus;   //lgh
    uint8_t      	ErrorAll;//�¹���
	uint8_t   		WarmAll;//�¹���
	uint32_t		ReadYXPtr;//��ң��ָ��
	uint32_t		ReadDYXPtr;//��˫��ң��ָ��
	uint32_t		ReadYXSoePtr;//��ң���¼�ָ��
	uint32_t		ReadDYXSoePtr;//��˫��ң���¼�ָ��
	uint32_t		ReadYCPtr;//��ң��ָ��
	uint32_t		ReadAitPtr;//����ʱ��ң��ָ��
	uint32_t		ReadBCDPtr;//����ʱ��ң��ָ��
	uint32_t		ReadDZPtr;//����ֵָ��
	uint32_t		ReadMrPtr;//���¹ʼ�ָ��
	
	uint8_t			Flag_Link_Ready;//��·������
	uint8_t			Flag_Brust_Send_Enable;//��������ͻ����Ϣ
	int 			fd;
	int 			sfd;//tcp server���ļ���������fd��������ļ�������
	TLogicBase    	*pLogicBase;
	int 			SoeSendNum;
	
	int				STATE_FLAG_INIT;
	int				STATE_FLAG_CALLALL;
	int				STATE_FLAG_DZCALLALL;
	int				STATE_FLAG_GROUP;
	int				STATE_FLAG_CLOCK;
	int				STATE_FLAG_TESTER;
	int 			STATE_FLAG_DELAY;
	int 			STATE_FLAG_YK;
	int 			YK_FROM_ID;//ң����Դ��ID��
	int 			ReSendNum;
	
	
	int				Flag_start_link;
	char			logbuf[100];

	uint8_t 			ReSendCnt;
	uint8_t 			ReSendFlag;
	uint32_t 			ReSendOldTime;
	uint32_t 			ReConnectOldTime;
	int 			ReSendDataLen;
	int				fcb;
	uint8_t 		Respon_Confirm;
	uint8_t 		flag_enpty_task;
	uint8_t 		flag_enpty_receive;
	uint8_t 		flag_enpty_timeout;
	uint8_t 		Flag_Set_DZ;	
	uint8_t 		Flag_Select_DZ;//��ֵԤ�ñ�־
	int				DZ_YZ_DEVID;//��ֵ�޸Ķ���ID
	uint8_t Flag[MAX_FLAGNO];
	uint32_t dwFlag[MAX_DWORDFLAGNO];

	/*MODBUS*/
	RTULocalMasterType *pModbusMaster;
	TCPLocalClientType *pModbusTcpMaster;
	ModbusData_T ModbusData;
}TDevie,*PTDevie;


 
extern TransTable_T *TransYXTable;
extern TransTable_T *TransYCTable;
extern TransTable_T *TransDZTable;
extern Modbus_TransTable_T *TransModbusCoidStatusTable;
extern Modbus_TransTable_T *TransModbusInputStatusTable;
extern Modbus_TransTable_T *TransModbusHoldingRegTable;
extern Modbus_TransTable_T *TransModbusInputRegTable;


extern	PTDevie gpDevice;
extern	TVars   gVars;

#pragma pack() //�����1�ֽڶ���


//======================================================
//	�ⲿ����������
//======================================================
void InitDbase(void);
int WriteYx(uint8_t DevID,uint32_t nPoint,uint8_t bStatus);
int WriteYxSoe(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,CP56Time2a_T *_Time);
int WriteYxSoeDB(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,uint8_t bResdStatus,CP56Time2a_T *_Time);
int WriteYc(uint8_t DevID,uint32_t nPoint,uint16_t bValue);
int WriteYc16(uint8_t DevID,uint32_t nPoint, uint16_t bValue);
int WriteYc32(uint8_t DevID,uint32_t nPoint, uint32_t bValue);
int WriteYxSoe(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,PCP56Time2a_T _Time);
int WriteYcSoeDB(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,uint8_t bResdStatus,uint16_t bValue,CP56Time2a_T *_time);

int ReadYx(uint32_t nIndex);
uint16_t ReadYcData16(uint32_t nIndex);
uint32_t ReadYcData32(uint32_t nIndex);
int WriteDzYz(int DevID,uint32_t nIndex, uint32_t bValue);
int SetRealDZFlag(uint16_t DevID, uint16_t nPoint, uint8_t flag);
int GetDZRealID(uint32_t nIndex);
int ScanBrustDZ(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustDZNum);
int WriteRealDz(uint8_t DevID,uint32_t nPoint, uint32_t bValue);
int SetDZFlag(uint32_t nIndex, uint8_t flag);
int ScanBrustYx(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustYXNum);
int ScanBrustYxSoe(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustYXNum);
uint32_t ReadDzYZData32(uint32_t nIndex);
int ReadYcFlag(uint16_t nIndex);
int ReadDzFlag(uint16_t nIndex);
uint32_t ReadDzData32(uint32_t nIndex);
void SetCallAllFlag(void);
void SetCallDZAllFlag(void);
int SetDZCallFlag(uint32_t nIndex, uint8_t flag);
int SetYCFlag(uint32_t nIndex, uint8_t Value);
int ReadDzCallFlag(uint16_t nIndex);
int SetYXFlag(uint16_t nIndex, uint8_t Value);
uint8_t ReadYxFlag(uint16_t nIndex);
uint32_t GetDevFlagValue(int    DevID, DWORD dwFlagNo);



#endif
