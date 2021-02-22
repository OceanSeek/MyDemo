#ifndef __siec101_2002_H__
#define __siec101_2002_H__

 

//头文件引用区      
//#include "CSlave.h"
//#include "cjptrlist.h"

BOOL Init_siec101_2002(int DevID);


#define USER_FLAG      32     //用户标志位区
#define USER_SFLAG   32     //用户标志位区 
#define USER_SDWORDFLAG		8		//用户区DWORD数据存储区
#define SYSTEM_SFLAG		0     //系统保留标志位区


#define SF_HaveBO         (SYSTEM_SFLAG+0)    //有遥控
#define SF_HaveBOReturn   (SYSTEM_SFLAG+1)    //有遥控返校
#define SF_HaveAO         (SYSTEM_SFLAG+2)    //有遥调
#define SF_HaveAOReturn   (SYSTEM_SFLAG+3)    //有遥调返校
#define SF_HaveTQ         (SYSTEM_SFLAG+4)    //有同期
#define SF_HaveTQReturn   (SYSTEM_SFLAG+5)    //有同期返校
#define SF_Time           (SYSTEM_SFLAG+6)    //需对时
#define SF_Proc           (SYSTEM_SFLAG+7)    //已处理标志
#define SF_HaveDO         (SYSTEM_SFLAG+8)    //有数字量输出
#define SF_HaveDOReturn   (SYSTEM_SFLAG+9)    //有数字量输出返校
#define SF_HaveCOE        (SYSTEM_SFLAG+10)    //有参数
//lgh--
#define SF_HaveDZ         (SYSTEM_SFLAG+14)    //有定值
#define SF_HaveDZReturn   (SYSTEM_SFLAG+15)    //有定值返校


typedef struct {

//内存块控制
	DWORD  dwMemBlockSize;   //公用数据处理缓冲区大小,用pGetMemBlock()取得该缓冲区首址

//通讯控制
//	DWORD  dwMaxSleepTime; //最长通讯中断时间
	DWORD  dwTxdRetryCount;  //重发数次
	DWORD  dwMaxErrCount;    //连续出错dwMaxErrCount次后通知数据库
    DWORD  dwTxdBufSize;     //发送缓冲区大小
	DWORD  dwRxdBufSize;     //接收缓冲区大小
	
//数据库个数控制
	DWORD  dwMinLogicDevNum;	 //最小逻辑库设备数
	DWORD  dwMinRealDevNum;	 //最小实际库设备数
	DWORD  dwGBAddress;      	 //广播报文地址
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


#define GRP_CINUM   32		//每组最多发送电度个数
#define GRPNUMCI	16		//电度组数 16*32=512

#define SF_RESET_PROCESS	(USER_SFLAG+1)	//复复位RTU标志

#define SF_RtuInitOk		(USER_SFLAG+32)   //RTU初始化结束
#define SF_SendInitFlag		(USER_SFLAG+33)	  //发送初始化结束标志

#define SF_CallData			(USER_SFLAG+34)		//目前处于召唤数据状态
#define SF_CallCmdFlag		(USER_SFLAG+35)		//响应总召唤命令
#define SF_ClockSynFlag		(USER_SFLAG+36)		//响应对钟命令		
#define SF_SetParaFlag		(USER_SFLAG+37)		//参数命令
#define SF_CallDelayFlag	(USER_SFLAG+38)
#define SF_ClockDifferFlag	(USER_SFLAG+39)

#define SF_FILE_FLAG		(USER_SFLAG+40)		//文件属性发送标志，建立连接后要发送文件属性帧，以便主板确定是上装，还是下装文件
#define SF_ReadCIFlag		(USER_SFLAG+41)		//read ci flag
#define SF_MirrorFlag		(USER_SFLAG+42)		//从站回答镜像
#define SF_HaveScanAiData	(USER_SFLAG+43)		//有扫描的遥测数据

#define SF_YkRtnInfoFlag	(USER_SFLAG+48)		//遥控返回标志
#define SF_YkExitFlag		(USER_SFLAG+49)		//遥控结束标志

#define SF_YtRtnInfoFlag	(USER_SFLAG+50)		//遥控返回标志
#define SF_YtExitFlag		(USER_SFLAG+51)		//遥控结束标志

#define SF_CALLDIR_FLAG		(USER_SFLAG+52)		//目录召唤标记
#define SF_FILE_SELECT		(USER_SFLAG+53)		//file select
#define SF_FILE_REQUEST		(USER_SFLAG+54)		//file request
#define SF_FILE_SENDING		(USER_SFLAG+55)		//file sending
#define SF_LAST_SEGMENT		(USER_SFLAG+56)		//last segment
#define SF_LAST_SECTION		(USER_SFLAG+57)		//last section

#define SF_CALLCIFLAG0		(USER_SFLAG+64)//总召唤电度起始标记16*32=512

#define SF_RXDCONTROL		(USER_SDWORDFLAG + 0) //接受到的控制字
#define SF_NEXT_FCBFCV		(USER_SDWORDFLAG + 1)

#define SF_FILE_CRC			(USER_SDWORDFLAG + 2) //接收到的文件的CRC, 下传文件时用的
#define SF_RTU_STATUS		(USER_SDWORDFLAG + 3)//

#define SF_FILE_NAME		(USER_SDWORDFLAG + 4)
#define SF_SECTION_NAME		(USER_SDWORDFLAG + 5)
#define SF_SEGMENT_NAME		(USER_SDWORDFLAG + 6)
#define SF_FILE_OFFSET		(USER_SDWORDFLAG + 7)
#define SF_FILEOBJ_ADDR		(USER_SDWORDFLAG + 8)



//宏定义区文件引用区
#define MAX_TXDFM_SIZE  256  //最大发送帧长
#define MAX_RXDFM_SIZE  256  //最大接收帧长
#define MIN_RXDFM_SIZE  5    //最小接收帧长

//帧控制域定义
#define BIT_DIR    0x80    //传输方向位
#define BIT_PRM    0x40    //启动报文位
#define BIT_FCB    0x20    //帧计数位
#define BIT_FCV    0x10    //帧计数有效位
#define BIT_CODE   0x0F    //功能码所占位

#define BIT_FCBFCV	0x30

//功能码定义
#define  RESREMLIN 0x00 //复位远方链路
#define  RESRTU    0x01 //复位远动终端
#define  SENDAT    0x03 //传送数据
#define  REQANS    0x08 //召唤要求访问位
#define  REQLINSTA 0x09 //请求远方链路状态
#define  CALFIRDAT 0x0A //召唤一级用户数据

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
	BYTE bSOF;//文件状态
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
    BYTE bStart;	//启动字符
    BYTE bControl;	//控制域
	BYTE bAddress;//链路地址域（子站站址）
    BYTE bChkSum;	//帧校验和
    BYTE bStop;		//结束字符
}TRxdFixFm, TTxdFixFm; //I101S规约的固定帧长结构


typedef struct 
{
    BYTE bStart1;		//启动字符
    BYTE bLength1;		//长度
    BYTE bLength2;		//重复长度
    BYTE bStart2;		//启动字符
    BYTE bControl;		//控制域
	BYTE bAddress1;		//链路地址域（子站站址）  可变 1 or 2
    BYTE bType;			//类型标识
    BYTE bDefinitive;	//结构限定词
	BYTE bReason;		//传送原因		可变 1 or 2
	BYTE bAddress2;		//公共地址（子站站址）可变 1 or 2
    BYTE bData;			//数据开始
}TRxdVarFm,TTxdVarFm;	//I101S规约的可变帧长结构

typedef union 
{
	TRxdFixFm fixFm;
	TRxdVarFm varFm;
}TSIEC101_2002RxdFm;		//101规约的接收帧结构（头部）

typedef union 
{
	TTxdFixFm fixFm;
	TTxdVarFm varFm;
}TSIEC101_2002TxdFm;			//规约的发送帧结构（头部）

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


//TxdIdle消息时应发送的帧类别TITF(m_Txd Idle Task Flag)


//class CSIEC101_2002 : public C_Slave  
//{
//        
//public:
//	CSIEC101_2002();
//    BOOL Init(void);//初始化
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
//	TSIEC101_2002RxdFm *pRxdFm; //接收帧头指针
//    TSIEC101_2002TxdFm *pTxdFm;//发送帧头指针
//
//	BOOL CheckAndRetry(BYTE bControl);
//
//	void SetALLDataFlag();
//	
//    void OnTimeOut(DWORD dwTimerID);
//    void OnBOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwBONo,DWORD dwResult);//遥控返校
//	void OnAOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwAONo,short AOValue,DWORD dwResult);//遥调返校
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
//	BOOL RxdReqLinkStatus();//请求链路状态
//	BOOL RxdClass1Data();//一级数据
//	BOOL RxdClass2Data();//二级数据
//	
//	BOOL RxdSetPara();
//	BOOL RxdSetPara2();
//	BOOL RxdSetPara3();
//	BOOL RxdClockSyn();
//	BOOL RxdCallAll();
//	BOOL RxdCallCI();//召唤冻结电度
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
//	BOOL Txd_ChangeAI13(); //发送变化遥测浮点数据
//	BOOL Txd_ChangeAI14();
//	BOOL Txd_ChangeAI36();//发送变化遥测长时标浮点数据
//	BOOL Txd_ChangeAI34();//发送变化遥测长时标normalisee数据
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
//	//发送报文处理函数
//	BYTE GetCtrCode(BYTE PRM,BYTE dwCode); //OK
//	BOOL Txd_Head(BYTE dwCode,BYTE Reason);//发送报文头
//	BOOL Txd_Tail(BYTE PRM,BYTE dwCode,BYTE Num);//发送报文尾
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

