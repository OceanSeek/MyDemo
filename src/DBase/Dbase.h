#ifndef __DBASE_H__
#define __DBASE_H__


#define MAX_FILE_LEN			(256*1024)  //长度大于该数的文件将不能被读出 
#define MAX_DEVEICE_NUM			150			//系统可以配置的总设备个数，包括逻辑设备和实际设备的总数 
#define MAX_ROW 					200			//每次读取表单最大数据量
#define MT_USER    0x26c   //实际库
#define MT_LOGIC   0x276   //逻辑库
//=============  通讯检测位定义  ===============
#define FM_SHIELD   	0xFFFF0000      //屏蔽字
#define FM_OK       	0x00010000      //检测到一个完整的帧
#define FM_ERR      	0x00020000      //检测到一个校验错误的帧
#define FM_LESS     	0x00030000      //检测到一个不完整的帧（还未收齐）
//遥控消息
#define PBOINFOSIZE  	8
//遥调消息
#define PAOINFOSIZE  	8
//同期消息
#define PTQINFOSIZE  	8

#define TP_AOSELECT    		1    //遥调预置
#define TP_AOOPERATE   		2    //遥调执行
#define TP_AOCANCEL    		3    //遥调撤消
#define TP_AODIRECT    		4    //遥调直接执行
#define MAX_FLAGNO    			256   	//每个模块最多的标志位数
#define MAX_DWORDFLAGNO			32		//每个模块最多的DWORD标志个数

#define SM_P(PVID) {}
#define SM_V(PVID) {}
  //========================================/|
 //PV操作								   /||
//========================================/ ||
#define MPV_BIBURST  	0    //遥信变位专用PV	||  ||
#define MPV_SOE      	1    //SOE专用PV	||  ||
#define MPV_GVARS    	2    //gVars专用PV	||  ||
#define MPV_OPENCOMM 	3    //任务专用PV	||  ||
#define MPV_LOGERR   	4    //LOGERR专用PV	||  ||
#define MPV_TIME     	5    //TIME专用PV	||  ||
#define MPV_MEM      	6    //MEM专用PV		||  ||
#define MPV_OI       	7    //OI专用PV		||  
#define MPV_TASK    	8	 //任务	
#define MPV_DBIBURST  	9    //双点遥信变位专用PV	||
#define MPV_DBISOE      10   //双点SOE专用PV	||  ||
#define MPV_MR 	     	11   //简报专用PV	||
#define MPV_AIT 	    12   //简报专用PV	||
#define MPV_NETDRV		13	//以太网驱动用
#define MPV_PROMPT		14	//显示信息用
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



//需要清0的全局变量定义

typedef struct	//I104S规约的可变帧长结构  
{
    unsigned char Start; 		//启动字符
    unsigned char Length; 		//长度
    unsigned char Control1;		 //控制域1
    unsigned char Control2; 	//控制域2
    unsigned char Control3; 	//控制域3
    unsigned char Control4; 	//控制域4
    unsigned char Style;   		//类型标识，7
    unsigned char Definitive; 	//结构限定词
    unsigned char Reason_L; 	//传送原因
    unsigned char Reason_H; 	//传送原因
    unsigned char Address_L; 	//公共地址（子站站址）
    unsigned char Address_H; 	//公共地址（子站站址）
    unsigned char AddInfom1;  	//信息体地址，第13字节
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
    
}TS104APDU;


typedef struct{
	uint32_t dwTaskNum; //任务总数
	uint32_t dwDevNum; //系统所接设备总数 
	uint32_t dwPortNum; //系统所接端口总数

//	SEM_ID adwPVID[MAX_PVNUM+1]; //PV操作ID
	uint32_t dwVarNum; //或查看变量的个数
//	TVarInfo VarInfo[MAX_VAR_NUM+1];
//用于判断任务是否已加载

//用于避免重复错误/警告信息	
//	uint32_t adwLogged[MAX_LOGGED_NUM+1];
	uint32_t dwTID_BI;
	uint32_t dwTID_JC;
	uint32_t dwTID_AO;
	uint32_t dwTID_TQ;
	uint32_t dwTID_BO;
	uint32_t dwTID_BCD;
	uint32_t dwTID_DCAI;
	uint32_t dwTID_PRINT;
	uint32_t dwAddress;//1725做采集模块时的地址
	uint32_t dwBHType; //用于系统模块的保护类型标识，如液晶显示。
	int8_t bTimeSyn;//时钟同步状态。	
//任务
	
	uint32_t dwTID_KEYBOARD;  //KEYBOARD任务ID
	uint32_t dwTID_DISPLAY;   //DISPLAY任务ID
	uint32_t dwTID_WatchBakPort;//主备口监视任务
	
	uint32_t dwTID_EtherNetDrv; //以太网驱动任务
	uint32_t dwTID_SerialDrv;   //串口驱动任务
	uint32_t dwTID_MFdkBusDrv;  //FDKBUS主站端驱动任务
	uint32_t dwTID_SFdkBusDrv;  //FDKBUS从站端驱动任务
	uint32_t dwTID_MCanBusDrv;  //CANBUS主站端驱动任务
	uint32_t dwTID_SCanBusDrv;  //CANBUS从站端驱动任务

	void *pFirstMalloc; //动态分配内存的起始链表
	void *pLastMalloc; //动态分配内存的结束链表

	void *pMemFree;   //动态释放内存的起始链表

//小片内存动态分配控制管理
	int8_t *pLittleRam; //小片内存未分配起始偏移量未分配起始偏移量
	uint32_t dwLittleRamSize; //小片内存未分配起始偏移量

//NVRAM
	uint32_t dwNvRamPtr; //NVRam未分配起始偏移量
	uint32_t dwNvRamID; //NVRam分配区的ID
	
//	TOIEWrap OIEWrap; //透明数据区
//	TCommBlock CommBlock; //通讯报文监视
//	TBIEvent BIEventInfo;//记录需要发遥信变位和SOE的任务
//	TBOControl BOControlMsg; 
//	TMREvent MREventInfo;	//记录需要发送事故简报的任务

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
	uint32_t YxWritePtr;//遥信写指针
	uint32_t DYxWritePtr;//双点遥信写指针
	uint32_t DZWritePtr;//定值写指针
	uint32_t YxSoeWritePtr;//遥信写指针
	uint32_t YxDSoeWritePtr;//双点遥信写指针
	uint32_t AitWritePtr;//带时标遥测写指针
	uint32_t BCDWritePtr;
	uint32_t MrWritePtr;//事故简报写指针
	uint32_t YcWritePtr;//遥测写指针
	uint8_t	Flag_ReSet;
}TVars;


typedef struct {
	uint32_t	value;		//32位定值
	uint16_t	nNo;		//转发点号
	uint8_t		flag;		//
	uint8_t		flag_call;	//总召唤标志
	uint32_t	YZ_value;	//预置值
	int			YZ_Caller;	//预置来源的设备ID号

	/*5512-VXWORKS*/
	DWORD dwNo;//转发点号
	WORD  wStatus;
	DWORD sValue; //
	/*5512-VXWORKS*/
}TBurstDZ;


#pragma pack(1)	//按1字节对齐


//标准指针操作结构
typedef struct{
	uint32_t dwWritePtr; //写指针
	uint32_t dwReadPtr;  //读指针
}TPtr;

typedef struct{			//分配在不掉电RAM中
	TPtr Ptr;
}TSoeWrap;

/*----------------------------------------------*/
/* 逻辑设备定值描述库定义                                                                 */
/*----------------------------------------------*/
typedef struct{                    //modify by lgh
    WORD        wRealID;                /*所在设备ID*/
    WORD        wOffset;                /*定值在实时库中的偏移量*/
    short	a;			 //系数A
	short	b;			 //系数B
	WORD	wFlag;
}TLogicDZ;


/*----------------------------------------------------------------------------------------*/
/* 定义基本时间格式                                                                          */
/*----------------------------------------------------------------------------------------*/
typedef struct {

	uint16_t MSecond;				 /*毫秒*/
	uint8_t Minute; 				/*分*/
	uint8_t Hour;					/*时*/
	uint8_t Day;					/*日*/
	uint8_t Month;					/*月*/
	uint8_t Year;					/*年*/
}TSysTime,*PTSysTime;


typedef struct {
	uint16_t  			nNo;			//点号
	uint8_t  			bStatus;		//状态 0:由合至分   非0：由分至合
	uint8_t  			bType;       	//0x0A-无  0x0B-三相电流 0x0C-电压 0x0D-频率 0x0E-零序
	IEC10X_DETECT_T		detect16;//16位遥测值
	IEC10X_DETECT_DW_T 	detect32;//32位遥测值
  	CP56Time2a_T stTime;    	//年、月、日、时、分、秒、毫秒
}TSysTimeSoe,*PTSysTimeSoe;

typedef struct {
	WORD 	 nNo;			//点号
	BYTE	 bStatus;		//状态 0x01:分; 0x02:合; 0x00:机构错误;	0x03：非法
	CP56Time2a_T stTime;    	//年、月、日、时、分、秒、毫秒
}TSysTimeDBISoe;

typedef struct {
	WORD 	 nNo;			//点号
	BYTE	 bStatus;		//状态 0x01:分; 0x02:合; 0x00:机构错误;	0x03：非法
	TSysTime stTime;    	//年、月、日、时、分、秒、毫秒
}TAbsTimeDBISoe;



typedef struct {
    WORD       Lo;                     /*低位系统时间*/                    
    DWORD        Hi;                     /*高位系统时间*/
}TAbsTime;

typedef struct {
	WORD		nNo;		//点号
	BYTE		bStatus;	//状态 0:由合至分   非0：由分至合
	TAbsTime	stTime;		//绝对时标
}TAbsTimeSoe;


typedef struct {
	WORD 	 	nNo;		//遥测点号
	short	 	sValue;		//越限遥测值
	TAbsTime	stTime;		//绝对时标
}TAbsTimeAit;


typedef struct {
//当前存的是命令信息
	WORD wFlag;	  		//任务状态  PBO_NULL, PBO_BUSY, PBO_WAITOPERATE
	DWORD dwAppID;    	//下发遥控的任务ID;
	DWORD dwNo;       	//实时库路号／点号
	DWORD dwAttrib;   	//属性 //状态（合分）TP_NULL TP_OPEN, TP_CLOSE,
	DWORD dwCommand;  	//动作 TP_YKSELECT TP_YKOPERATE  TP_YKCANCEL TP_YKDIRECT
	DWORD dwResult;   	//报文返回的遥控返校状态 PBO_SUCESS  PBO_FAIL
	DWORD dwRetNo;    	//下发的路号／点号
	DWORD dwRetDevID;	//下发遥控的device ID
	DWORD dwQualitier;	//控制命令的限定 考虑到兼容性，动作类型
	DWORD dwExeCount;    	//遥控执行次数
	DWORD dwCTime;		// 撤销时间s
	DWORD dwTimeH;		//继电器闭合时间，
	DWORD dwTimeL;		//继电器打开时间，

	BYTE  Info[8];
}TBO;

/*----------------------------------------------------------------------------------------*/
/* 实际设备遥控描述库定义                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char	   szName[20];	/*遥控对象名称*/
    DWORD      dwBOID;      //高16位为属性  0:无  1：CLOSE 合 2：Open分
    			        	//低16位为开关号/路号
	BYTE		ucType;		//控制类型，现定义为如下5种：
							//0：NUL操作。
							//1：脉冲ON，按周期循环输出ON-OFF，最后保持在OFF态。
							//2：脉冲OFF，按周期循环输出OFF-ON，最后保持在ON态。
							//3：栓锁ON，持续输出ON态。
							//4：栓锁OFF，持续输出OFF态。
							//5~255：保留
							//注：脉冲输出方式中的周期，由下面的wOnTime及wOffTime
							//指定。
	BYTE		ucCount;	//脉冲输出方式中的输出周期数
	WORD		wOffTime;	//脉冲输出方式中的OFF态持续时长，ms。为与原参数兼容，
							//所以把wOffTime放在了前面
	BYTE 		ucAutoResetTime;	//自动撤消时间(s)
	WORD	   	wLastTime;	/*继电器动作持续时间*/
}TRealBO;


//事故简报
typedef struct { //malfunction report
	WORD 	 	wAddr;			//故障模块地址
	DWORD 	 	dwBhType;		//保护类型
	WORD 	 	wAttribute;		//简报属性：测试/正常
	WORD 	 	wMrNo;			//简报序号
	DWORD		dwTripType;		//跳闸类型
								//bit0--I段跳；
								//bit1--II段跳；
								//bit2--III段跳；
								//bit3--低周跳；
								//bit4--过负荷跳；
								//bit5--后加速跳；
								//bit6--重合闸；
								//bit7--重瓦斯跳；
								//bit8--380V零序保护跳；
								//bit9--过电压跳；
								//bit10--低电压跳；
								//bit11--不平衡电压跳；
								//bit12--不平衡电流跳；

	TSysTime 	stTime;     	//故障起始时间 年、月、日、时、分、秒、毫秒
	DWORD		dwTripTime;		//动作时间 相对系统时间的毫秒数
	DWORD		dwTwjTime;		//TWJ时间 相对系统时间的毫秒数
	DWORD		dwChzTime;		//重合闸时间 相对系统时间的毫秒数
	DWORD		dwHwjTime;		//TWJ时间 相对系统时间的毫秒数
	WORD		wRsv[4];		//保留；

	WORD		wDataNum;		//测量值个数；
//	short		sData[16];		//测量值内容；
	DWORD		dwData[16];		//测量值内容；
}TSysTimeMr;


//--------------------------------------
// 变位遥测描述库定义						
//--------------------------------------
typedef struct {
	IEC10X_DETECT_F_T	detect32_F;//32位浮点遥测值
	IEC10X_DETECT_DW_T	detect32;//32位遥测值
	IEC10X_DETECT_T 	detect16;//16位遥测值	
	uint16_t			nNo;			//点号
	uint16_t			nIndex;				//索引号
	uint8_t 			bType;			//0x0A-无	0x0B-三相电流 0x0C-电压 0x0D-频率 0x0E-零序
	uint8_t			flag;			//0:无 1:突发标志 2:总召唤标志
//	TSysTime stTime;		//年、月、日、时、分、秒、毫秒

	/*5512-VXWORKS*/
	DWORD dwNo;//转发点号
	WORD  wStatus;
	DWORD sValue; //
	TSysTime stTime;
	/*5512-VXWORKS*/
}TBurstAI;


//--------------------------------------
// 变位遥信描述库定义						
//--------------------------------------
typedef struct {
	CP56Time2a_T 	stTime;        			//数据库在检测到变位后自动生成的系统时间
	uint16_t		nNo;					//发送号
	uint16_t		ID;						//索引号
	uint8_t			bStatus;				//状态
	uint8_t		flag;					//0:无 1:突发标志 2:总召唤标志
	uint8_t			FlagSoe;				//0:无 1:遥信事件突发标志
}TBurstBI,*PTBurstBI;

//--------------------------------------
// 变位遥信记录						
//--------------------------------------
typedef struct {
	CP56Time2a_T stTime;
	uint8_t		bStatus;			//状态
	uint16_t	point;				//发送点号
}TBurstYXRecord,*PTBurstYXRecord;

typedef struct {
	DWORD dwNo;
	WORD wStatus;
	DWORD dwValue;
	TSysTime stTime;
}TBurstBcd;


//--------------------------------------
// 变位遥信记录						
//--------------------------------------
typedef struct {
	CP56Time2a_T stTime;
	uint32_t	Value32;//32位遥测值
	uint16_t	point;				//发送点号
}TBurstYCRecord,*PTBurstYCRecord;

//--------------------------------------
// 变位遥信环型缓冲区定义													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//写指针
}TBurstAIWrap;

//--------------------------------------
// 变位遥信环型缓冲区定义													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//写指针
}TBurstBIWrap;

//--------------------------------------
// 变位遥信环型缓冲区定义													              */
//--------------------------------------
typedef struct {
	TPtr        Ptr;						//写指针
}TBurstDBIWrap;

//--------------------------------------
// 变位遥信描述库定义						
//--------------------------------------
typedef struct {
	uint32_t		nNo;				//发送号
	uint8_t		bStatus;				//状态
	CP56Time2a_T 	stTime;        			//数据库在检测到变位后自动生成的系统时间
}TBurstDBI;

//--------------------------------------
// 变位遥信SOE描述库定义						
//--------------------------------------
typedef struct {
	uint8_t		nFlag;					//设备号
	uint8_t		bStatus;				//状态
	uint8_t		bResdStatus;			//读取状态,0：未读；1：已读。
	uint32_t	nPoint;					//设备内点号
	uint32_t	nIndex;					//转发点号
	TSysTime 	stTime;        			//存储变位时间
}TBurstBISoe;

//--------------------------------------
// 变位遥测SOE描述库定义						
//--------------------------------------
typedef struct {
	uint8_t		nFlag;					//设备号
	uint8_t		bStatus;				//状态
	uint8_t		bResdStatus;			//读取状态,0：未读；1：已读。
	uint16_t	bValue;					//设备内点号
	uint32_t	nPoint;					//设备内点号
	uint32_t	nIndex;					//转发点号
	TSysTime 	stTime;        			//存储变位时间
}TBurstAISoe;



//--------------------------------------
// 转发库定义						
//--------------------------------------
typedef struct {
	uint16_t		ID;					//索引号	
	uint16_t		wRealID;			//接入设备ID号
	uint16_t		nPoint;				//设备内点号
	uint16_t 		nIndex;        		//转发点号
	
}TransTable_T,*PTransTable_T;

typedef struct {
	uint16_t		ID;					//索引号	
	uint16_t		wRealID;			//接入设备ID号
	uint16_t		nPoint;				//设备内点号
	uint16_t 		nIndex;        		//转发点号
	uint16_t 		RegAddr;        	//寄存器地址
	uint16_t 		SlaverAddr;        	//寄存器地址
}Modbus_TransTable_T,*PModbus_TransTable_T;


/*----------------------------------------------------------------------------------------*/
/* 逻辑设备遥测描述库定义                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wConfig;            //控制字节
    WORD        wRealID;            //被引用的AI所属的智能设备的ID
    WORD        wOffset;            //被引用的AI在所属智能设备的实际库中的偏移量
    short       a;                  //遥测系数
    short       b;
    short       c;
    WORD 		    wSQZ;		   			    //死区值 高位1表示相对死区，0,绝对死区
    WORD		    wFlag;				      //数据发送时做标记用的
    DWORD		    sOldValue;			    //用于变化遥测的记录
    WORD		    wOldFlag;
    TSysTime	  stTime;
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
	DWORD dwBcdBurstReadPtr;//变化bcd读指针
	DWORD dwBcdBurstWritePtr;//写指针
	TBurstBcd		*pBurstBcd;
}TBurstBcdWrap;


/*----------------------------------------------------------------------------------------*/
/* 逻辑设备遥信描述库定义                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wRealID;            //被引用的AI所属的智能设备的ID
    WORD        wOffset;            //被引用的AI在所属智能设备的实际库中的偏移量
    TSysTime	  stTime;
	WORD		wFlag;		/*规约发送数据时使用*/
}TLogicBI;

/*----------------------------------------------------------------------------------------*/
/* 逻辑设备遥信描述库定义                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
    WORD        wRealID;                /*该BI所属智能设备的ID*/
    WORD        wOffset;                /*该BI在所属智能设备实时库中的偏移量*/
    WORD        wConfig;                /*控制字节*/
										/*D0=1 引用固态数据 =0 不引用固态数据*/
										/*D1=1 引用事件数据 =0 不引用事件数据*/
										/*D2=1 引用原始数据 =0 不引用原始数据*/
										/*D3=1 引用转换后的虚拟信息 =0 不引用虚拟信息*/
										/*D4=1 事故总标志有效 =0 无效 */
										/*D5~7保留*/
	WORD	wFlag;						/*规约发送数据时使用*/
}TLogicDBI;



typedef struct{
    WORD	wRealID;                /*该DD所属设备的ID号*/
    WORD	wOffset;                /*该DD在设备实际库中的偏移量*/
    WORD	wNo;
    WORD	wConfig;                /*控制字节*/
										/*  D0=1 为正向电度 =0 为负向电度*/
										/*  D1~7保留*/
    short   a;
    short   b;
    short   c;
	BYTE	bFreezeFlag;			/*冻结标志，同时防止数据没有读走而丢数*/
	BYTE	bReset;					/*bReset=1发送差值, bReset=0发送总数*/
    WORD    wFreezeTime;			/*冻结时间*/
	WORD	wCountTime;
	DWORD	dwTotalValue;			/**/
	WORD	wStatus;				/*属性*/
	DWORD	dwFreezeValue;			/*冻结的数据*/
	WORD	wFlag;					/*发送标志*/
	TSysTime stTime;				/*冻结的时间*/
}TLogicCI;



typedef struct TREFLEX {
    DWORD       dwDevNo;                 /*引用该数据的逻辑库设备编号*/
    DWORD       dwNo;                    /*该数据在逻辑设备库中的偏移量*/
    struct TREFLEX *pNext;
}TReflex, *PTREFLEX;

typedef struct{
    WORD        wRealID;            /*该BCD所属智能设备的ID*/
    WORD        wOffset;            /*该BCD在所属智能设备实时库中的偏移量*/
	WORD		wFlag;				/*规约发送数据时用*/
}TLogicBCD;

typedef struct {
	WORD wFlag;	        //任务状态  PAO_NULL, PAO_BUSY, PAO_WAITOPERATE
	WORD wNo;			//遥控路号
	DWORD dwCommand;	//动作 TP_AOSELECT TP_AOEXECUTE  TP_AOCANCEL TP_AODIRECT
	DWORD dwResult;
	DWORD dwDoValue;	//数字量输出值
	DWORD dwOnTime;		//on 时间
	DWORD dwOffTime;	//off 时间
	DWORD dwCount;	    //次数
	WORD wNeedRetFlag;      //是否需要数字量输出返校信息标志1:need,0:NoNeed
	BYTE bInfo[PAOINFOSIZE];//保留，遥于保留特殊信息
//	TDOMsg DOMsg;           //数字量输出原信息
	BYTE Info[8];
}TDO;

typedef struct {
	WORD wFlag;	        //任务状态  PAO_NULL, PAO_BUSY, PAO_WAITOPERATE
	WORD wNo;			//遥控路号
	DWORD dwCommand;	//动作 TP_AOSELECT TP_AOEXECUTE  TP_AOCANCEL TP_AODIRECT
	DWORD dwResult;
	short sAoValue;	        //遥调值
	WORD wNeedRetFlag;      //是否需要遥调返校信息标志1:need,0:NoNeed
	BYTE bInfo[PAOINFOSIZE];//保留，遥于保留特殊信息
//	TAOMsg AOMsg;           //遥调原信息
	BYTE  Info[16];
}TAO;

/*----------------------------------------------------------------------------------------*/
/* 实际设备遥调描述库定义                                                                 */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char		szName[20];		/*YT对象名称*/
    WORD        wControl;               /*rsv*/
    WORD        nID;                    /*遥调动作号*/
	WORD		wMaxValue;              /*满值*/		/*a,b,c 分别对应系数和修正值*/
	BYTE		bResetTime;
	BYTE		bRsv1;
	WORD		rsv2;
}TRealAO;


/*----------------------------------------------------------------------------------------*/
/* 实际设备遥测描述库定义                                                                     */
/*----------------------------------------------------------------------------------------*/
typedef struct {
	char		szName[20];				/* 遥测名称*/
	DWORD		dwControl;				/*控制字节*/
	WORD		wForceToZero;
	short   a;            /*a,b,c为系数,完成运算y=x*a/b+c*/
	short   b;
	short   c;
	short   sHigh;        /* 上限 */
	short   sLow;         /* 下限 */
	short		sHighBack;				/* 遥测故障报警值*/
	short		sLowBack;				  /*遥测故障值*/
	WORD		wDeadBand;				/* 遥测死区值 ,D15=1相对死区值,=0为绝对死区值 */
	WORD		wSmoothFactor;		/*故障动作时延*/
	WORD	  wType;					/*直流输入类型*/
	TReflex *pReflex;   	/*逻辑库索引交叉表*/
}TRealAI;


typedef struct {			//带时标遥测队列;不掉电ram
	TPtr Ptr;
}TAitWrap;

//====================================
//	带时标遥测（越限遥测）相关结构
//====================================
typedef struct {
	uint16_t 	 	    nNo;		//转发点号
	uint16_t 	 	    ID;		//索引号
	uint8_t        bStatus;    //越限状态0：越上限1：越下限2：越上限恢复3：越下限恢复
	uint16_t	 	    sValue;		//越限遥测值
	uint32_t	 	    sValue32;		//越限遥测值
	TSysTime 	  stTime;    	//年、月、日、时、分、秒、毫秒
}TSysTimeAit;

//====================================
//		逻辑库相关结构
//====================================
typedef struct{
    DWORD 			    *pAddCI;			//电度的累加值,是计算值，放于RAM中
	DWORD			      *pLogicOldBcd;		/*保存的旧的bcd的值*/
    TLogicDZ        *pLogicDZ;          /*定值逻辑属性库头指针,分配在动态RAM区*/    
    TLogicAI        *pLogicAI;          /*遥测逻辑属性库指针,分配在动态RAM区*/
    TLogicBI        *pLogicBI;          /*遥测逻辑属性库指针,分配在动态RAM区*/
    TLogicCI        *pLogicCI;          /*遥测逻辑属性库指针,分配在动态RAM区*/
    TLogicBCD       *pLogicBCD;         /*BCD逻辑属性库指针,分配在动态RAM区*/
    TLogicDBI       *pLogicDBI;         /*遥信逻辑属性库指针,分配在动态RAM区*/
    TBurstAIWrap     BurstAIWrap;		/*变化遥测头指针,分配在动态RAM区*/
	TBurstBcdWrap burstBcdWrap;
}TLogicBase;

//--------------------------------------
// 实际设备遥信描述库定义                                                                 */
//--------------------------------------
typedef struct {
	char	szName[20];	/*遥信对象名称*/
	DWORD 	dwControl;	//D0~15 为I/O单元使用的控制位; 与主控板无关
						//D16~31为主控使用的控制位；与I/O单元无关
						//D0 = 1 产生不带时标的事件数据； = 0 不产生
						//D1 = 1 产生带时标的事件数据； = 0不产生
						//D2 = 1 第一点取反； = 0 第一点不取反
						//D3 = 1 发送   = 0 不发送
						//D4 = 1 事故总有效  = 0 无效
						//D5 = 1 是事故总信号
						//D6 = 1 第二点取反； = 0 第二点不取反
						//D7~ D8 保留
						//D9~D15  chatter filter 的变化次数，最大为127次

						//D16 = 1 由主控产生不带时标的事件数据； = 0 不产生
						//D17 = 1 由主控产生带时标的事件数据； = 0不产生
						//D18 = 1 由主控进行第一点取反操作； = 0 不取反
						//D19 = 1 由主控进行第二点取反操作； = 0 不取反
						//D20~31 保留

	WORD 	wFilterTime;	//Debounce防抖时间，也就是最小的有效脉宽，ms
	WORD 	wDINo;			//双点中的第一点的接线位置序号，自0计起，必须为偶数
							//对于双点中的第二点，其接线位置必须是与第一点紧邻的
							//下一点，不在属性参数中明示。
	WORD 	wChatterFilterTime; 	//chatter filter 的监测时间，ms，最大为60000ms
	WORD 	wDPSynTime;				//双点同步时间，ms
	DWORD	dwChatterNum;			//允许变化个数
	DWORD	dwLockoutNum;			//稳定周期个数
	WORD	wSoeEnable;  			//用于第一次收到BI时禁止产生SOE
	WORD	wCosEnable;
	TReflex *pReflex;   
}TRealDBI;
//--------------------------------------
// 实际设备遥信描述库定义                                                                 */
//--------------------------------------
typedef struct {
	char	     szName[20];			/*遥信对象名称*/
	DWORD        dwControl;             	/*D0=1 由数据库管理器产生不带时标的事件数据；=0 不产生*/
						/*D1=1 由数据库管理器产生带时标的事件数据；=0不产生*/
						/*D2=1 进行取反操作；=0 不取反*/
						/*D3=1 是事故总信号*/
						/*D4~7 类*/
						/*  =0 单点遥信*/
						/*  =1 双点遥信*/
	WORD	    wDINo;	  			/*相对应的数字采集点号*/
	WORD	    wFilterTime;  			/*消颤周期*/
	WORD	    wChatterFilterTime;			//chatter filter 的监测时间，10ms单位
	DWORD	    dwChatterNum;			//允许变化个数
	DWORD	    dwLockoutNum;			//稳定周期个数
	WORD		  wSoeEnable;  			//用于第一次收到BI时禁止产生SOE 
	WORD		  wCosEnable;
	TReflex   *pReflex;   
}TRealBI;
//--------------------------------------
// 实际设备电度描述库定义
//--------------------------------------
typedef struct {
    char	szName[20];				/*电度名称*/
    WORD    wControl;      	/*电度控制字,D2=1:发送,D2=0:不发送，D3=1:脉冲输入,D3=0:积分电度*/
    WORD    a;              /*系数*/
    WORD    b;              /*系数*/
    DWORD   dwOrgValue;     /*初值*/
    WORD	  wDINo;
    WORD	  wDPSynTime;		//双点同步时间，ms，仅对双点脉冲有效
}TRealCI;


typedef struct{
	DWORD					 wIOControl;	    	  /*new! I/O单元使用的控制字节*/
	DWORD					 wSControl;	    	    /*主处理器使用的控制字节*/
//	TRealDI	       *pRealDI;	          /*遥信类型头指针，分配在动态RAM区*/
  	TRealAI        *pRealAI;            /*遥测实际库属性指针,分配在动态RAM区*/
	TRealBI        *pRealBI;            /*遥信实际库属性头指针,分配在动态RAM区*/
	TRealCI        *pRealCI;            /*电度实际库属性头指针,分配在动态RAM区*/
// 	TRealBO        *pRealBO;            /*遥控属性库,分配在动态RAM区*/
//  	TRealAO        *pRealAO;            /*遥调属性库,分配在动态RAM区*/
//	TRealSync      *pRealSync;          /*同期属性库,分配在动态RAM区*/
//  	TTransDataAttr *pTransAttr;         /*透明数据描述库,分配在动态RAM区*/
	TRealDBI       *pRealDBI;           /*双点遥信实际库属性头指针,分配在动态RAM区*/
//  TRealDO        *pRealDO;            /*数字量输出属性库,分配在动态RAM区*/
//  TRealBCD       *pRealBCD;           /*BCD实际库属性指针,分配在动态RAM区*/
//  TRealDZ        *pRealDZ;            /*定值属性库,分配在不掉电RAM区*/
//  TRealMR        *pRealMR;            /*事故简报属性库,分配在不掉电RAM区*/
//  TRealSELF      *pRealSELF;          /*自检信息属性库,分配在不掉电RAM区*/
}TRealBase;

typedef struct{			//分配在不掉电RAM中
	TPtr Ptr;
}TDBISoeWrap;

typedef struct  
{
	WORD wStartAddr;//信息体起始地址；默认值是0
	BYTE bStatus; //bit0, bit1 =1容许，=0不容许发送
	BYTE bStatus1;//=1
	BYTE bStatus2;//2短时标，30长时标
	BYTE bClass;//优先级
}TData_t;

typedef struct   //规约io 
{
	unsigned short BufSize;		//缓冲区大小
	unsigned short ReadPtr;		//读指针
	unsigned short OldReadPtr;  //读指针
	unsigned short WritePtr;	//写指针
	WORD	Address;			//地址码(主要用于发送)
	DWORD	Flag;				//控制码(主要用于发送)
	unsigned char  *Buf;		//缓冲区
}TCommIO;

typedef struct{

    uint16_t	ID;
	uint16_t	ParentID;		//模块ID
    uint16_t   	Address;		//自己的地址
	uint16_t	AIStart;	//遥测偏移地址
	uint16_t	BIStart;	//遥信偏移地址
	uint16_t	DZStart;	//定值偏移地址
	uint16_t	DBIStart;	
	uint16_t	BOStart;
	uint16_t	YMStart;
	uint16_t 	AINum;			//遥测个数
	uint16_t	BINum;			//遥信个数
	uint16_t 	dwCINum;		//电度个数
	uint16_t	BONum;			//遥控个数
	uint16_t	AONum;			//遥调个数
	uint16_t	DZNum;			//定值个数
	uint16_t	SyncNum;		//同期个数
  	uint16_t 	TransNum;   	//透明数据关键字个数
	uint16_t 	DONum;			//数字量输出个数
	uint16_t 	dwBCDNum;			//BCD个数
	uint16_t 	DBINum;			//双点遥信个数

//	/*MODBUS*/
//	RTULocalMasterType *pModbusMaster;
//	ModbusData_T ModbusData;
}TWQ900,*PTWQ900;

typedef struct{

    uint16_t	ID;
	uint16_t	ParentID;		//模块ID
    uint16_t   	Address;		//自己的地址
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
	BYTE bBalance;//不用
	BYTE bLinkAddr;//链路层地址
	BYTE bAsduAddr;//公用地址
	BYTE bInfoAddr;//信息体地址
	BYTE bCauseTrans;//传送原因
	BYTE bTestMode;//测试模式,bit7:1:测试模式，0：正常模式
	BYTE bCounterMode;// bit0,bit1 countermode, 0,1,2Modea, Modeb, modec
	WORD wCylicDataNum;//周期数据个数,默认是0,遥测的周期数据
	WORD wScanTime;//扫描时间
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
	TData_t bdo;//二进制输出
	TData_t tq;
	TData_t mr;//保护
	TData_t file;//文件
	TData_t rsv[8];
	//	struct TData_t data [10];	//14个数据结构，其中最后4个为保留
	WORD wCheckSum;	//累加和，使用低16位，不考虑进位
}TIEC101Cfg;


typedef struct{

//============ 操作 ============//

    int16_t (*ReadAI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus); 	//读一个遥测
    int16_t (*ReadAI32)(uint32_t dwDevID,uint32_t dwAINo, IEC10X_DETECT_F_T* pValue); 	//读一个遥测
    void  (*WriteAI)(uint32_t dwDevID,uint32_t dwAINo,int16_t sValue);       	//写一个遥测

    uint8_t  (*ReadDBI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//读一个双点遥信
    void  (*WriteDBI)(uint32_t dwDevID,uint32_t dwAINo,uint8_t bValue);        	//写一个双点遥信

    uint8_t  (*ReadBI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//读一个遥信
    void  (*WriteBI)(uint32_t dwDevID,uint32_t dwAINo,uint8_t bValue,uint8_t bType,uint8_t bValue0,uint8_t bValue1,uint8_t bValue2,uint8_t bValue3);      //写一个遥信	

    uint32_t (*ReadCI)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);    //读一个电度
    void  (*WriteCI)(uint32_t dwDevID,uint32_t dwAINo,uint32_t dwValue);        //写一个电度

    uint32_t (*ReadBCD)(uint32_t dwDevID,uint32_t dwAINo, uint16_t* pStatus);	//读一个BCD
    void  (*WriteBCD)(uint32_t dwDevID,uint32_t dwAINo,uint32_t dwValue);       //写一个BCD

    uint32_t (*ReadDZ)(uint32_t dwDevID,uint32_t dwDZINo, uint16_t* pStatus); 	//读一个定值
    void  (*WriteDZ)(uint32_t dwDevID,uint32_t dwDZNo,uint32_t sValue,uint16_t pStatus); //写一个定值

    int (*SetYK)(int DevID, uint8_t Type, uint16_t COT, uint8_t YKData); //发送遥控信息
    int (*Receive)(int DevID, uint8_t *buf, uint16_t len); //接收数据
	int	(*Task)(int DevID);//循环任务
	int	(*OnTimeOut)(int DevID);//定时任务
	int	(*SetDZ)(int DevID);//定值激活
	
    
    uint16_t	ID;
	uint16_t	DeviceID;		//模块ID
    char		Name[64];		//模块名称
    uint16_t   	SAddress;		//自己的地址
    uint16_t   	MAddress;		//对方的地址
	char*	    SelfIP;			//自己的IP地址
	char*	    DirfIP;			//设备的IP地址
	uint16_t 	AINum;			//遥测个数
	uint16_t	BINum;			//遥信个数
	uint16_t 	dwCINum;		//电度个数
	uint16_t	BONum;			//遥控个数
//	uint16_t	AONum;			//遥调个数
	uint16_t	DZNum;			//定值个数
	uint16_t	SyncNum;		//同期个数
  	uint16_t 	TransNum;   	//透明数据关键字个数
	uint16_t 	DONum;			//数字量输出个数
	uint16_t 	dwBCDNum;			//BCD个数
	uint16_t 	YBNum;			//定值个数
    uint16_t 	MRDeviceNum;	//产生事故简报的设备数：实际库固定为1；逻辑库可选
	uint16_t 	DBINum;			//双点遥信个数
	uint16_t 	SELFNum;		//自检信息设备个数
	uint16_t 	MaxSoeBuffer;	// 单点soe/变化遥信区个数
	uint16_t 	MaxDBISoeBuffer;// 双点遥信soe/变化遥信区个数
	uint16_t 	MaxAitBuffer;	// 单点soe/变化遥信区个数
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
	char 		Com[32];		//接口类型:网口或串口
	uint32_t 	SerialID;		//串口编号
	uint32_t 	NetID;			//网口编号
	char		TcpType[32];		//tcp类型:client or server
	char 		Protocol[32];		//协议类型
	char		Type[32];		//设备类型:终端或主站
	char 		dwType;//设备类型:终端或主站5512
	uint32_t	dwControl;
//    uint16_t    YCNum;			//遥测个数
//    uint16_t    YXNum;			//遥信个数
//    uint16_t    YKNum;			//遥控个数
    uint16_t	YMNum;			//遥脉个数
    char		IP[32];
    uint16_t    PORT;			//IP端口号
    char		UDP_Dest_IP[32];
    uint16_t    UDP_Dest_PORT;			
	uint32_t	SerialBault;
	uint32_t	SerialDataBit;
	uint32_t	SerialStopBit;
	uint16_t	SerialParity;
	YK_INFO_T 	YKInfo;
	uint8_t 	*pSendBuf;//发送数据缓冲区
	uint8_t 	*pRecBuf;//接收数据缓冲区
	
	//实际库与逻辑库都有的数据
	TSoeWrap		*pSoeWrap;			//SOE指针 NVRAM
	TSysTimeSoe 	*pSysTimeSoe;		//SOE库 NVRAM
	
	TRealBase     	*pRealBase; 
    TBurstAIWrap    *pBurstAIWrap;      /*遥信变位指针,分配在不掉电RAM区*/
    TBurstAI    	*pBurstAI;       	/*遥测变位库,分配在不掉电RAM区*/
	
    TBurstBIWrap    *pBurstBIWrap;      /*遥信变位指针,分配在不掉电RAM区*/
    TBurstBI    	*pBurstBI;       	/*遥信变位库,分配在不掉电RAM区*/

    TBurstDBIWrap   *pBurstDBIWrap;     /*遥信变位指针,分配在不掉电RAM区*/
    TBurstDBI    	*pBurstDBI;       	/*遥信变位库,分配在不掉电RAM区*/

    TBurstDZ    	*pBurstDZ;       	/*定值库,分配在不掉电RAM区*/


    TAitWrap 		*pAitWrap;			//带时标遥测指针 NVRAM
    TSysTimeAit 	*pSysTimeAit;		//带时标遥测库 NVRAM

    TDBISoeWrap 	*pDBISoeWrap;		//DOUBLE POINT YX SOE指针 NVRAM
    TSysTimeDBISoe 	*pSysTimeDBISoe;	//DOUBLE POINT YX SOE库 NVRAM

    DWORD 		*pAI;               /*遥测实时库指针,分配在不掉电RAM中,逻辑库指针分配在普通RAM*/
	uint16_t		*pAIStatus;			/*遥测值的状态*/
    uint16_t 		*pBI;               /*遥信实际数据库头指针,分配在不掉电RAM区*/
    uint16_t 		*pDBI;              /*遥信实际数据库头指针,分配在不掉电RAM区*/

    uint32_t 		*pCI;               /*电度库头指针,分配在不掉电RAM区*/
	uint16_t		*pCIStatus;			/*电度属性*/
	
    uint32_t 		*pBCD;              /*BCD头指针,分配在不掉电RAM区*/
	uint16_t		*pBCDStatus;		/*bcd status ptr*/
	
	uint16_t		*pBO;				/*遥控值*/
	uint16_t		*pBOStatus;			/*遥控属性*/
	uint16_t		*pAO;				/*遥调值*/
	uint16_t		*pAOStatus;			/*遥调属性*/
	uint32_t 		*pDZ;               /*定值变位库,分配在不掉电RAM区*/
	uint16_t		*pDZStatus;   //lgh
    uint8_t      	ErrorAll;//事故总
	uint8_t   		WarmAll;//事故总
	uint32_t		ReadYXPtr;//读遥信指针
	uint32_t		ReadDYXPtr;//读双点遥信指针
	uint32_t		ReadYXSoePtr;//读遥信事件指针
	uint32_t		ReadDYXSoePtr;//读双点遥信事件指针
	uint32_t		ReadYCPtr;//读遥信指针
	uint32_t		ReadAitPtr;//读带时标遥测指针
	uint32_t		ReadBCDPtr;//读带时标遥测指针
	uint32_t		ReadDZPtr;//读定值指针
	uint32_t		ReadMrPtr;//读事故简报指针
	
	uint8_t			Flag_Link_Ready;//链路已连接
	uint8_t			Flag_Brust_Send_Enable;//允许发送突发信息
	int 			fd;
	int 			sfd;//tcp server绑定文件描述符，fd存放侦听文件描述符
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
	int 			YK_FROM_ID;//遥控来源的ID号
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
	uint8_t 		Flag_Select_DZ;//定值预置标志
	int				DZ_YZ_DEVID;//定值修改对象ID
	uint8_t Flag[MAX_FLAGNO];
	uint32_t dwFlag[MAX_DWORDFLAGNO];

	/*MODBUS*/
	RTULocalMasterType *pModbusMaster;
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

#pragma pack() //解除按1字节对齐


//======================================================
//	外部函数声明区
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
